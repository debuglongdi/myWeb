#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <strings.h>
using namespace mymuduo;
using namespace mymuduo::net;

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localaddr)
{
    int ret = ::bind(sockfd_, (sockaddr*)localaddr.getSockAddr(), sizeof(sockaddr_in));
    if(ret < 0)
    {
        LOG_FATAL("bind sockfd:%d error\n",sockfd_);
    }
}
void Socket::listen()
{
    int ret = ::listen(sockfd_, 1024);
    if( ret < 0)
    {
        LOG_FATAL("listem sockfd:%d failed\n",sockfd_);
    }
}
int Socket::accept(InetAddress* peeraddr)
{
    sockaddr_in addr;
    socklen_t len;
    bzero(&addr, sizeof addr);
    int connfd = ::accept(sockfd_,(sockaddr*) &addr, &len);
    if(connfd >= 0)
    {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

/// @brief 关闭写端
void Socket::shutdownWrite()
{
    int ret = ::shutdown(sockfd_, SHUT_WR);
    if(ret < 0)
    {
        LOG_ERROR("shutdownWrite error\n");
    }
}

//使用sockopt()设置socket选项
void Socket::setTcpNoDelay(bool on)
{
    int optval = on?1:0;
    int ret = ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,sizeof optval);

}
void Socket::setReuseAddr(bool on)
{
    int optval = on?1:0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,&optval,static_cast<socklen_t> (sizeof optval));
}
void Socket::setReusePort(bool on)
{
    int optval = on?1:0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,&optval,sizeof optval);
}
void Socket::setKeepAlive(bool on)
{
    int optval = on?1:0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,&optval,sizeof optval);
}