#include "../include/Channel.h"
#include "../include/EventLoop.h"
#include "../include/Logger.h"
#include <sys/epoll.h>

using namespace std;
using namespace mymuduo;
using namespace mymuduo::net;

const int Channel::kReadEvent = EPOLLIN | EPOLLPRI ;
const int Channel::kWriteEvent = EPOLLOUT;
const int Channel::kNonEvent = 0;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index_(-1)
    , tied_(false)
{
}
Channel::~Channel()
{
    // if(loop_->isInLoopThread()){

    // }
}

/**
 * tie什么时候被调用？
*/
void Channel::tie(const std::shared_ptr<void> &obj)
{
    //tie_是弱智能指针，观察是否可以绑定强智能指针obj
    tie_ = obj;
    tied_ = true;
}

/**
 * 当改变channel所表示的events事件后，update负责在poller里面更改fd相应的事件
 * 因为channel与poller都是属于eventloop的，它们互不所属。
*/
void Channel::update()
{
    //通过channel所属的EventLoop,调用poller相应的方法，注册fd事件
    loop_->updateChannel(this);
}
/**
 * 在channel所属的EventLoop中，把当前的channel删除了
*/
void Channel::remove()
{
    loop_->removeChannel(this);
}


void Channel::handleEvent(Timestamp receiveTime)
{
    if(tied_){
        std::shared_ptr<void> guard;
        //提升tie_;
        //std::weak_ptr<void> tie_;
        guard = tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }else{
         handleEventWithGuard(receiveTime);
    }

}

/**
 * 根据poller通知的channel发生的具体事件，由channel负责调用具体的回调操作
*/
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("Channel handleEvent revents%d\n", revents_);
    //关闭
    if((revents_ & EPOLLHUP) && !( revents_ & EPOLLIN ))
    {
        if(closeCallback_){
            closeCallback_();
        }
    }
    //错误事件处理
    if(revents_ & EPOLLERR){
        if(errorCallback_){
            errorCallback_();
        }
    }
    //读事件
    if(revents_ & (EPOLLIN | EPOLLPRI ))
    {
        if(readCallback_){
            readCallback_(receiveTime);
        }
    }
    //写事件
    if( revents_ & EPOLLOUT)
    {
        if(writeCallback_){ writeCallback_(); }
    }

}