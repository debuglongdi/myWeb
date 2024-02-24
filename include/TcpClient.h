#pragma once 
#include "noncopyable.h"
#include "TcpConnection.h"
#include <functional>
#include <memory>
#include <mutex>

namespace mymuduo
{
namespace net
{
class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;
class TcpClient : noncopyable
{
public:
    TcpClient(EventLoop* loop,
            const InetAddress &serverAddr,
            const std::string &nameArg);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const;
    // {
    //     // std::unique_lock<std::mutex> lock(mutex_);
    //     std::unique_lock<std::mutex> lock(mutex_);
    //     return connection_;
    // }
    EventLoop* getLoop() const { return loop_; }
    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }

    const std::string& name() const
    { return name_; }

    ///set connection callback
    ///not thread safe
    void setConnectionCallback(const ConnectionCallback &cb)
    { connectionCallback_ = std::move(cb); }
    void setMessageCallback(const MessageCallback &cb)
    { messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    { writeCompleteCallback_ = std::move(cb); }



private:
    //not thread safe but in loop
    void newConnection(int sockfd);
    //not thread safe but in loop;
    void removeConnection(const TcpConnectionPtr &conn);
    EventLoop* loop_;
    //不要暴露连接器
    ConnectorPtr connector_;
    const std::string name_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    //建立连接失败后是否尝试重新连接
    std::atomic_bool retry_;
    std::atomic_bool connect_;
    int nextConnId_;
    //conn连接互斥访问
    std::mutex mutex_;
    TcpConnectionPtr connection_;
};

} // namespace net
} // namespace mymuduo
