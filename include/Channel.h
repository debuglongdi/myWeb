#pragma once
#include "../include/noncopyable.h"
#include "../include/Timestamp.h"
#include <functional>
#include <memory>

namespace mymuduo
{
namespace net
{
    class EventLoop;
    /**
     * Channel:通道，对IO事件的fd的抽象
    */
    class Channel : noncopyable
    {
    public:
        using EventCallBack = std::function<void()>;
        using ReadEventCallBack = std::function<void(Timestamp)>;
        Channel(EventLoop *loop, int fd);
        ~Channel();

        //设置回调函数对象
        void setReadCallback(ReadEventCallBack cb)
        { readCallback_ = std::move(cb); }
        void setWriteCallback(EventCallBack cb)
        { writeCallback_ = std::move(cb); }
        void setCloseCallback(EventCallBack cb)
        { closeCallback_ = std::move(cb); }
        void setErrorCallback(EventCallBack cb)
        { errorCallback_ = std::move(cb); }

        /// Poller监听到channel上有事件发生后通知EventLoop，然后EventLoop调用channel的handleEvent事件处理函数
        /// handleEvent会根据事件类型调用不同的处理函数
        void handleEvent(Timestamp receiveTime);

        //防止channel被手动销毁后，channel还在执行回调操作
        void tie(const std::shared_ptr<void> &);

        int fd() const { return fd_; }
        int events() const { return events_; }
        void set_revents(int revt) { revents_ = revt; }
        

        //设置fd上要监控的事件（底层是用位掩码）
        void enableReading(){ events_ |= kReadEvent; update(); }
        void disableReading(){ events_ &= ~kReadEvent; update(); }
        void enableWriting(){ events_ |= kWriteEvent; update(); }
        void disableWriting(){ events_ &= ~kWriteEvent; update(); }
        void disableAll(){ events_ = kNonEvent; update(); }

        //返回fd上感兴趣的事件
        bool isWriting() const { return events_ & kWriteEvent; }
        bool isReading() const { return events_ & kReadEvent; }
        bool isNonEvent() const { return events_ == kNonEvent; }

        //for Poller
        int index() const { return index_;}
        void set_index(int idx){ index_ = idx; }
        //channel的属主是哪个EventLoop(one loop per thread)
        EventLoop* ownerLoop() { return loop_;}

        void remove();
    private:
        void update();
        void handleEventWithGuard(Timestamp receiveTime);
        static const int kNonEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;//channel所在的事件循环
        const int fd_;//fd, 通道对象抽象的fd
        int events_;//注册fd感兴趣的事件
        int revents_;//返回的具体发生的事件
        int index_; //-1:未注册到poller中 1：注册到poller中，2：从poller中删除

        //与智能指针一起使用，防止循环引用
        std::weak_ptr<void> tie_;
        bool tied_;

        ReadEventCallBack readCallback_;
        EventCallBack writeCallback_;
        EventCallBack closeCallback_;
        EventCallBack errorCallback_;
    };    
} // end namespace net
} // end namespace mymuduonamespace net
