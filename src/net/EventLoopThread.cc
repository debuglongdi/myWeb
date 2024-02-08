#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace mymuduo;
using namespace mymuduo::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                const std::string &name)
                : loop_(nullptr)
                , exiting_(false)
                , thread_(std::bind(&EventLoopThread::threadFunc,this), name)
                , mutex_()
                , cond_()
                , callback_(std::move(cb))
{ }
EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        //退出事件循环
        loop_->quit();
        //等待线程退出
        thread_.join();
    }
}
//开启新线程
EventLoop* EventLoopThread::startLoop()
{
    //开启线程，会执行构造thread的回调threadFunc()
    thread_.start();
    
    EventLoop* loop = nullptr;
    std::unique_lock<std::mutex> lock(mutex_);
    while(loop == nullptr)
    {
        cond_.wait(lock);
    }
    return loop;
}


//在开启Thread新线程要做的事，创建一个独立的EventLoop
void EventLoopThread::threadFunc()
{
    //独立的EventLoop，在栈区管理
    EventLoop loop;
    if(callback_)
    {
        //要对loop进行的回调操作
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    //开启事件循环
    loop.loop();
    //事件循环结束了
    //所有代码都退出了
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}