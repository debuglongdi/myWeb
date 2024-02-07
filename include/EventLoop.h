#pragma once
#include "noncopyable.h"
#include <functional>
#include "Timestamp.h"
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include "CurrentThread.h"

namespace mymuduo
{
namespace net
{   
class Channel;
class Poller;
/**
 * 事件循环类，主要包含了两个模块
 * Poller (epoll的抽象,监听一堆channel)
 * Channel (文件描述符【fd以及感兴趣的事件】的抽象)
*/
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    //开启事件循环
    void loop();
    //退出事件循环
    void quit();

    Timestamp pollRetrunTime() const { return pollReturnTime_; }

    //在当前loop中执行回调
    void runInLoop(Functor cb);
    //把cb放入队列中，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);
    
    // 用来唤醒loop所在线程
    void wakeup();

    //EventLoop 的 poller_的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    //判断EventLoop对象是否在自己的线程里
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    //处理wakeupfd上的读事件
    void handldRead();
    void doPendingFunctors();//执行回调

    std::atomic<bool> looping_;
    std::atomic<bool> quit_; //标识退出循环
    
    const pid_t threadId_; //记录loop所在的线程id
    Timestamp pollReturnTime_; //poller返回事件发生的时间点
    std::unique_ptr<Poller> poller_;

    //使用eventfd()系统调用
    int wakeupFd_;//当mainLoop获取一个新用户的channel,通过轮询算法选择一个subloop(woker thread),通过改变量唤醒subloop处理
    std::unique_ptr<Channel> wakeupChannel_;
    using ChannelList = std::vector<Channel*>;
    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    //vector线程不安，使用mutex互斥变量控制互斥访问
    std::mutex mux_;
    std::atomic<bool> callingPendingFunctors_; //标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;//存储当前loop是否有需要执行的回调操作
};
    
} // end namespace net
} // end namespace mymuduo
