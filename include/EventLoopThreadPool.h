#pragma once
#include "noncopyable.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "Logger.h"

//
namespace mymuduo
{
namespace net
{
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* baseLoop, const std::string &nameArg);
    ~EventLoopThreadPool();
    //设置线程池数量（不包括mainLoop）
    void setThreadNum(int numThread){ numThreads_ = numThread; }
    //开启所有线程池种的EventLoop
    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    //使用轮询获取下一个EventLoop
    EventLoop* getNextLoop();

    //获取所有的线程的EventLoop
    std::vector<EventLoop*> getAllLoop();

    bool started() const { return started_; }
    const std::string& name() { return name_; }
private:
    //用户创建的mainLoop
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    //subReactor的数目
    int numThreads_;
    int next_;
    //所有的工作线程suReactor(subLoop)
    std::vector<std::unique_ptr<EventLoopThread> > threads_;
    //所有工作线程对应的EventLoop
    std::vector<EventLoop*> loops_;
};

} // namespace net
} // namespace mymuduo
