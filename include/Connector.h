#pragma once
#include "noncopyable.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Logger.h"

#include <functional>
#include <memory>
#include <atomic>

namespace mymuduo
{
namespace net
{

/// @brief 主动连接套接字
class Connector : noncopyable,
                public std::enable_shared_from_this<Connector>
{
public:
    using NewConnectionCallback = std::function<void(int)>;
    Connector(EventLoop* loop, const InetAddress &serverAddr);
    ~Connector();

    void setNewConnectionCallbac(const NewConnectionCallback &cb)
    { newConnectionCallback_ = cb; }

    void start();
    //must be called int loop thread
    void restart();
    void stop();

    const InetAddress& serverAddress(){ return serverAddr_; }
    
private:
    enum States
    {
        kDisConnected,
        kConnecting,
        kConnected
    };
    void setStates(States s) { state_ = s; }
    //设置重试时间
    static const int kMaxRetryDelayMs = 30*1000;
    static const int kInitRetryDelayMs = 500;
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    InetAddress serverAddr_;
    bool connect_;
    std::atomic_int state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    //重试间隔时间
    int retryDelayMs_;
};

} // namespace net   
} // namespace mymuduo
