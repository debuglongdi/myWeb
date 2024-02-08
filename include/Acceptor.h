#pragma once 
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"
#include <functional>
#include <memory>

namespace mymuduo
{
namespace net
{
class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress& addr)>;
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();
    void setNewConnectionCallback(NewConnectionCallback &cb)
    {
        newConnectionCallback_ = std::move(cb);
    }
    void listen();
    bool listenning() const { return listenning_; }
private:
    void handleRead();

    //mainloop 这个是专门监听连接的那个
    EventLoop* loop_;
    //下面两个都是对同一个fd的封装
    Socket acceptSocket_;
    Channel acceptChannel_;
    //新连接来了后做的回调
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};

} // namespace net
} // namespace mymuduo
