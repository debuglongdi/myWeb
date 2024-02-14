#pragma once

#include "noncopyable.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include <functional>
#include <atomic>
#include <unordered_map>

namespace mymuduo
{
namespace net
{

/**
 * 对外提供的服务端编程接口
*/
class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum Option
    {
        kNoReusePort,
        kReusePort,
    };
    TcpServer(EventLoop * loop,
              const InetAddress& listenAddr,
              const std::string& nameArg,
              Option option = kNoReusePort);//默认不重用port
    ~TcpServer();
    const std::string& ipPort() const { return ipPort_; }
    const std::string& name() const { return name_; }
    EventLoop* getLoop() const {return loop_; }

    //设置处理事件的线程数目,subLoop
    void setThreadNum(int numThreads);

    
    ///线程池
    std::shared_ptr<EventLoopThreadPool> threadPool()
    { return threadPool_; }

    //开启服务器监听
    void start();

    //设置回调
    void setThreadInitCallback(const ThreadInitCallback& cb)
    { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb)
    { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb)
    { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    { writeCompleteCallback_ = cb; }

private:
    //not thread safe but in loop
    void newConnection(int sockfd, const InetAddress& peerAddr);
    //thread safe
    void removeConnection(const TcpConnectionPtr& conn);
    //not thread safe but in loop
    void removeConnectionInLoop(const TcpConnectionPtr& conn);


    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;//用户定义的loop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;//运行在mainLoop，监听新连接事件
    std::shared_ptr<EventLoopThreadPool> threadPool_;// one loop per thread

    //有新连接的回调
    ConnectionCallback connectionCallback_;
    //有读写消息回调
    MessageCallback messageCallback_;
    //消息写完的回调
    WriteCompleteCallback writeCompleteCallback_;
    //线程初始化回调
    ThreadInitCallback threadInitCallback_;
    std::atomic_int started_;

    int nextConnId_;
    //保存所有的连接
    ConnectionMap connections_;

};
    
} // namespace net
} // namespace mymuduo
