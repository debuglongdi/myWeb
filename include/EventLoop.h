#pragma once
#include "../include/noncopyable.h"
namespace mymuduo
{
namespace net
{   
    /**
     * 事件循环类，主要包含了两个模块
     * Poller (epoll的抽象,监听一堆channel)
     * Channel (文件描述符【fd以及感兴趣的事件】的抽象)
    */
    class EventLoop : noncopyable
    {
    private:
        /* data */
    public:
        EventLoop(/* args */);
        ~EventLoop();
    };
    
} // end namespace net
} // end namespace mymuduo
