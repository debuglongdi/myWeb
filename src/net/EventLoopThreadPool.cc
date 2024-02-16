#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "Logger.h"

using namespace mymuduo;
using namespace mymuduo::net;

//
EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string &nameArg)
    : baseLoop_(baseLoop)
    , name_(nameArg)
    , started_(false)
    , numThreads_(0)
    , next_(0)
{}
EventLoopThreadPool::~EventLoopThreadPool()
{
    //不用管理loop,它由栈管理
}
//开启所有线程池中的EventLoop
void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;
#ifdef MYMUDUO_DEBUG
    LOG_DEBUG("numThreads_=%d\n",numThreads_);
#endif
    for(int i = 0; i < numThreads_; ++i)
    {
#ifdef MYMUDUO_DEBUG
    LOG_DEBUG("################threadPool START########\n");
#endif
#ifdef MYMUDUO_DEBUG
    LOG_DEBUG("EventLoopThreadPool::start START THREAD [%d]\n",i+1);
#endif
        char buf[name_.size()+32];
        snprintf(buf, sizeof buf, "%s%d",name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb,buf);
#ifdef MYMUDUO_DEBUG
    LOG_DEBUG("################threadPool push1########\n");
#endif
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        //启动线程
#ifdef MYMUDUO_DEBUG
    LOG_DEBUG("################threadPool push2########\n");
#endif
        //被阻塞在这里了，需要解决
        loops_.push_back(t->startLoop());
#ifdef MYMUDUO_DEBUG
    LOG_DEBUG("################threadPool END########\n");
#endif
    }
    //用户没有设置多线程,只有一个mainloop
    if(numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}
//使用轮询获取下一个EventLoop处理事件
EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop = baseLoop_;
    if(!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if(next_ >= (int)loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}
//获取所有的线程的EventLoop
std::vector<EventLoop*> EventLoopThreadPool::getAllLoop()
{
    if(loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
    
}