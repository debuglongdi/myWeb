#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <functional>
#include <memory>

using namespace mymuduo;
using namespace mymuduo::net;

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &name,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(CHECK_NOTNULL(loop)), name_(name), socket_(new Socket(sockfd)), channel_(new Channel(loop, sockfd)), localAddr_(localAddr), peerAddr_(peerAddr), state_(kConnecting), reading_(true), highWaterMark_(64 * 1024 * 1024)
{
  // 在channel上注册回调处理函数，事件发生时会被调用
  channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
  channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
  LOG_INFO("%s:%d TcpConnection::ctor TcpConnection::TcpConnection\n", __FILE__, __LINE__);
  socket_->setKeepAlive(true);
}
TcpConnection::~TcpConnection()
{
  LOG_INFO("%s:%d TcpConnection::~ctor TcpConnection::TcpConnection fd=%d state=%s\n", __FILE__, __LINE__, channel_->fd(), stateToString());
}

void TcpConnection::handleRead(Timestamp reciveTime)
{
  int saveErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
  if (n > 0)
  {
    // 有读事件发生，读取数据后调用用户设置的回调，下面的回调必须设置
    if (messageCallback_ == nullptr)
    {
      LOG_FATAL("TcpConnection::handleRead please set messageCallback_\n");
    }
    messageCallback_(shared_from_this(), &inputBuffer_, reciveTime);
  }
  else if (n == 0) // 读事件发生但是没有数据，说明关闭了
  {
    handleClose();
  }
  else // 出现错误
  {
    errno = saveErrno = 0;
    LOG_ERROR("TcpConnection::handleRead \n");
    handleError();
  }
}

void TcpConnection::handleWrite()
{

  if (channel_->isWriting()) // fd可写
  {
    int saveErrno = 0;
    ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);

    if (n > 0)
    {
      if (outputBuffer_.readableBytes() == 0) // 数据发送完了
      {
        channel_->disableWriting();
        if (writeCompleteCallback_)
        {
          loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
        }

        if (state_ == kDisconnecting)
        {
          shutdownInLoop();
        }
      }
    }
    else
    {
      LOG_ERROR("TcpConnection::handleWrite \n");
    }
  }
  else // 可写事件发生但else不能写
  {
    LOG_ERROR("TcpConnection::handleWrite fd=%d is down no more writing \n", channel_->fd());
  }
}
void TcpConnection::handleClose()
{
  LOG_INFO("fd=%d state=%s\n", channel_->fd(), stateToString());
  setState(kDisconnected);
  TcpConnectionPtr conn(shared_from_this());

  // 连接断开回调
  if (connectionCallback_)
  {
    connectionCallback_(conn);
  }
  // 断开连接回调
  //  if(closeCallback_ == nullptr)
  //  {
  //    LOG_ERROR("you need set closeCallback_\n");
  //  }
  if (closeCallback_)
  {
    closeCallback_(conn);
  }
}
void TcpConnection::handleError()
{
  // 使用getsockopt()获取sockfd的错误码
  int optval;
  socklen_t optlen = sizeof optval;
  int err = 0;
  if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
  {
    err = errno;
  }
  else
  {
    err = optval;
  }
  LOG_ERROR("TcpConnection::handleError fd=%d err=%d\n",channel_->fd(), err);
}

const char *TcpConnection::stateToString() const
{
  switch (state_)
  {
  case kDisconnected:
    return "kDisconnected";
  case kConnecting:
    return "kConnecting";
  case kConnected:
    return "kConnected";
  case kDisconnecting:
    return "kDisconnecting";
  default:
    return "unknown state";
  }
}

void TcpConnection::shutdownInLoop()
{

}

