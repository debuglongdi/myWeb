#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"
#include <sys/eventfd.h>
#include <errno.h>

using namespace mymuduo;
using namespace mymuduo::net;

namespace
{
//防止一个线程创建多个EventLoop
__thread EventLoop* t_loopInThisThread = nullptr;
//设置默认超时时间
const int kPollTimeMs = 60000;

//创建wakeupfd,用来notify(唤醒)subRecator
int createEventfd()
{
    //通知suReactor处理channel
    int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evfd < 0)
    {
        LOG_FATAL("eventfd error %d\n", errno);
    }
    return evfd;
}

} // namespace


EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , threadId_(CurrentThread::tid())
    , poller_(Poller::newDefaultPoller(this))
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this, wakeupFd_))
    , currentActiveChannel_(nullptr)
    , callingPendingFunctors_(false)
{   
#ifdef MYMUDUO_DEBUG
    LOG_DEBUG("EventLoop create %p in thread %d\n",this, threadId_);
#endif
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop created %p in thread %d\n",t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }
    //设置wakeupfd的事件类型和发生事件后的回调操作
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handldRead, this)
    );
    //每个loop线程都监听自己的wakeupFd的读事件，该fd是eventfd，专门用来通信的，写数据和读书据都很快
    //这样其他的线程可以向该fd写数据来让该线程立刻启动，不然一般它都在阻塞等待监听的channel有就绪事件发生或者超时
    wakeupChannel_->enableReading();
}
EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

//开启事件循环
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("start looping %p\n",this);
    while(!quit_){
        activeChannels_.clear();
        //监听两类channel,一种是client的fd,一个是wakeupfd(######wakeupfd是必须监听的########，EventLoop之间通信、互相唤醒)
        //阻塞等待监听的channels(fd的抽象)上有就绪事件发生或者超时了
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        //Poller监听哪些channel发生事件了，然后上报给EventLoop,EventLoop通知channel处理相应的事件
        for(Channel* channel : activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }

        //执行当前EventLoop事件循环需要处理的回调操作
        /**
         * mainLoop accept接受新用户的链接（channel(this,fd)），然后把该channel分发给subLoop(wokrer)处理
         * minaLoop事先注册一个回调cb,使用wakeup()唤醒subloop执行下面的回调
         * 
        */
        doPendingFunctors();
    }
}
//退出事件循环
/**
 * 自己线程中quit直接退出
 * 调用其他loop的quit,那么你需要唤醒其他Eventloop线程（）,就像conditional_variable cv.notifyall();
*/
void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::handldRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if(n != sizeof(one))
    {
        LOG_ERROR("EventLoop::hadleRead() read %lu bytes instead of 8\n",n);
    }
}

//在当前loop中执行回调
void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}
//把cb放入队列中，唤醒loop所在的线程，执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mux_);
        pendingFunctors_.emplace_back(std::move(cb));
    }
    //不是在当前loop中，则需要唤醒对应的线程
    //或者callingPendingFunctors_==true 说明当前loop在执行上一轮的回调，所以也需要wakeup(),
    //等其执行完后(不会阻塞在poller_->poll(),因为wakeup()向其监听的wakeupfd中写数据了，它就变成读就绪，epoll就会立刻返回)，
    //又可以继续执行下一轮了
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

// 用来唤醒loop所在线程,只需要向wakeupfd写入数据，它就变成读就绪了
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes error\n");
    }
}

//EventLoop 的 poller_的方法
void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

//执行回调
void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    //使用局部变量将pendingFunctors_中待处理的cb copy过来，然后mainloop可以继续向其中写入
    {
        std::unique_lock<std::mutex> lock(mux_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor & functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}