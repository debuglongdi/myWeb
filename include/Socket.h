#pragma once
#include "noncopyable.h"

namespace mymuduo
{
namespace net
{
class InetAddress;

class Socket : noncopyable
{
public:
    explicit Socket(int fd):sockfd_(fd)
    {}
    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress &localaddr);
    void listen();
    //返回建立连接后通信的conn_sockfd
    int accept(InetAddress* peeraddr);

    /// @brief 关闭写端
    void shutdownWrite();

    //使用sockopt()设置tcp的属性
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    int sockfd_;
};

} // namespace net
} // namespace mymuduo
