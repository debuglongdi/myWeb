#include "Connector.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include "SockOps.h"
#include <thread>

using namespace mymuduo;
using namespace mymuduo::net;

// 在类的实现文件中
const int mymuduo::net::Connector::kMaxRetryDelayMs;
const int mymuduo::net::Connector::kInitRetryDelayMs;

int createNonBlocking()
{
    //ipv4的创建非阻塞的套接字，可以用0
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen sockfd create error\n",__FILE__,__FUNCTION__, __LINE__);
    }
    return sockfd;
}


Connector::Connector(EventLoop* loop, const InetAddress &serverAddr)
                    : loop_(loop)
                    , serverAddr_(serverAddr)
                    , connect_(false)
                    , state_(kDisConnected)
{
    LOG_INFO("Connector: ctor\n");
}
Connector::~Connector()
{
    LOG_INFO("Connector: dtor\n");
}


void Connector::start()
{
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop,shared_from_this()));
}

void Connector::startInLoop()
{
    if(connect_)
    {
        connect();
    }
    else 
    {
        LOG_INFO("do not connect\n");
    }
}

void Connector::connect()
{
    //创建非阻塞sockfd
    int sockfd = createNonBlocking();
    int ret = ::connect(sockfd, (sockaddr*)serverAddr_.getSockAddr(), sizeof(sockaddr_in));

    int saveErrno = (ret == 0)?0:errno;

    switch (saveErrno)
  {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      LOG_ERROR("========================connected===================================\n");
      connecting(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      LOG_ERROR("=============================retry==================================\n");
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_ERROR("connect error in Connector::startInLoop %d \n",saveErrno);
      ::close(sockfd);
      break;

    default:
      LOG_ERROR("Unexpected error in Connector::startInLoop %d \n",saveErrno);
      ::close(sockfd);
      // connectErrorCallback_();
      break;
  }
}

void Connector::connecting(int sockfd)
{
    //重新设置channel
    LOG_ERROR("========================the server has been connected===================================\n");
    //设置已经连接了
    setStates(kConnecting);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(
        std::bind(&Connector::handleWrite, shared_from_this())
    );
    channel_->setReadCallback(
        std::bind(&Connector::handleError,shared_from_this())
    );

    //对该套接字的写事件感兴趣，如果可写，说明可以连接了
    channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    //现在还不能移除，因为还在Channel::handleEvent
    loop_->queueInLoop(std::bind(&Connector::resetChannel, shared_from_this()));
    return sockfd;
}
void Connector::resetChannel()
{
    //使用智能指针，销毁对象
    channel_.reset();
}


void Connector::handleWrite()
{
    if(state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
         if (err)
        {
            LOG_ERROR("Connector::handleWrite - SO_ERROR =%d \n",err);
            retry(sockfd);
        }
        else if (sockets::isSelfConnect(sockfd))
        {
            LOG_ERROR("Connector::handleWrite - Self connect\n");
            retry(sockfd);
        }
        else//连接成功
        {
            setStates(kConnected);
            if(connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                ::close(sockfd);
            }
        }
    }
    else
    {
        LOG_INFO("what happend? \n");
    }
}
void Connector::handleError()
{
    LOG_ERROR("Connector::handleError state=%d\n",(int)state_);
    if (state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        LOG_ERROR("SO_ERROR = %d\n",err);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    sockets::close(sockfd);
    setStates(kDisConnected);
    if (connect_)
    {
        LOG_INFO("Connector::retry - Retry connecting to %s in  %d milliseconds. \n",serverAddr_.toIpPort().c_str(), retryDelayMs_);
        //TODO 设置超时重试
        std::this_thread::sleep_for(std::chrono::milliseconds(50000));
        loop_->runInLoop(std::bind(&Connector::startInLoop, shared_from_this()));//这个是错误的
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        LOG_INFO("do not connected \n");
    }
}


//must be called int loop thread
void Connector::restart()
{
    // loop_->assertInLoopThread();
    setStates(kDisConnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}
void Connector::stop()
{
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, shared_from_this())); // FIXME: unsafe
}


void Connector::stopInLoop()
{
     if (state_ == kConnecting)
    {
        setStates(kDisConnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}



