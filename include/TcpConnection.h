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
    void send(const std::string& message);
    void send(Buffer* buf);
    // void sendInLoop(const void* data, size_t len);
    // void send(const void* message, int len);
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
    //设置状态码
    void setState(StateE s) { state_ = s; }
    
    void handleRead(Timestamp reciveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    

    const char* stateToString() const;

/**
 * : loop_(CHECK_NOTNULL(loop))
 * , name_(name)
 * , socket_(new Socket(sockfd))
 * , channel_(new Channel(loop, sockfd))
 * , localAddr_(localAddr)
 * , peerAddr_(peerAddr)
 * , state_(kConnecting)
 * , reading_(true)
 * , highWaterMark_(64 * 1024 * 1024)
*/
    EventLoop* loop_;
    const std::string name_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_ ;
    //枚举类型
    std::atomic_int state_;
    bool reading_;
    //水位线，超过水位线停止发送
    size_t highWaterMark_;
    
    //一些回调函数
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    Buffer inputBuffer_; //接受数据的缓冲区
    Buffer outputBuffer_;//发送数据的缓冲区
};
    
} // namespace net
} // namespace mymuduo
