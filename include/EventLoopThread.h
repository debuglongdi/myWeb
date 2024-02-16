#pragma once
#include "noncopyable.h"
#include "Thread.h"
#include <mutex>
#include <condition_variable>
#include <functional>
#include "Logger.h"


//
namespace mymuduo
{
namespace net
{
class EventLoop;


class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback& cb=ThreadInitCallback(),
                    const std::string &name = std::string());
    ~EventLoopThread();
    //开启新线程
    EventLoop* startLoop();
private:
    //开启Thread新线程要做的事
    void threadFunc();

private:
    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

} // namespace net
} // namespace mymuduo
