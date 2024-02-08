#include "Acceptor.h"
#include "Socket.h"
#include "Logger.h"
#include "InetAddress.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>

using namespace mymuduo;
using namespace mymuduo::net;

namespace
{
int createNonBlocking()
{
    //创建非阻塞的套接字
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen sockfd create error\n",__FILE__,__FUNCTION__, __LINE__);
    }
    return sockfd;
}
} // namespace


Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createNonBlocking())
    , acceptChannel_(loop, acceptSocket_.fd())
    , listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    //注册回调,处理读事件
    acceptChannel_.setReadCallback(
        std::bind(&Acceptor::handleRead,this)
    );
}
Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    //开始监听
    acceptSocket_.listen();
    //mainLoop使用poller监听该套接字上的读事件，若有读事件，那就有新用户连接上了
    acceptChannel_.enableReading();

}
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    //接受一个连接
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0)
    {
        if(newConnectionCallback_)
        {
            //轮询找到一个工作线程（EventLoop）来处理新接受的连接
            newConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d Acceptor::handleRead() error\n",__FILE__,__FUNCTION__, __LINE__);
        if(errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d fd limit reached! error\n",__FILE__,__FUNCTION__, __LINE__);
        }
    }

}