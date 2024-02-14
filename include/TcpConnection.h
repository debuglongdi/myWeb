#pragma once
#include <memory>
#include <string>
#include <atomic>
#include "noncopyable.h"
#include "Callbacks.h"
#include "InetAddress.h"
#include "Buffer.h"

namespace mymuduo
{
namespace net
{
class Channel;
class EventLoop;
class Socket;

/**
 * TcpServer => Acceptor => 有一个新用户连接，通过accept()拿到连接的connfd
 * connfd->打包成TcpConnection 设置回调=》打包成Channel =>注册到Poller上=》(监听到事件就绪)执行Channel的回调 
*/
class TcpConnection : noncopyable,public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                const std::string &name,
                int sockfd,
                const InetAddress &localAddr,
                const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    //发送数据:c++11 void send(string&& message);
    void send(const void* message, int len);
    //关闭当前连接
    void shutdown();

    // void shutdownInLoop();

    //设置callback;
    void setConnectionCallback(const ConnectionCallback &cb)
    { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb)
    { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb)
    { highWaterMarkCallback_ = cb; }
    void setCloseCallback(const CloseCallback &cb)
    { closeCallback_ = cb; }

    //连接建立
    void connectEstablished();
    //连接销毁
    void connectDestroyed();


private:
    enum StateE{
        kDisconnected,kConnecting,kConnected, kDisconnecting
    };
    void handleRead(Timestamp reciveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    //设置状态码
    void setState(StateE s) { state_ = s; }

    const char* stateToString() const;

    EventLoop* loop_;
    const std::string name_;
    //枚举类型
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_ ;

    //一些回调函数
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    //水位线，超过水位线停止发送
    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};
    
} // namespace net
} // namespace mymuduo