#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <functional>
#include <memory>
#include <string>

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
                // 发送完数据，就对channel上的写就绪不感兴趣了，要设置disableWriting()
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
    if (closeCallback_)//TcpServe设置的断开连接的回调
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
    LOG_ERROR("TcpConnection::handleError fd=%d err=%d\n", channel_->fd(), err);
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

void TcpConnection::send(const std::string &message)
{
    // 已经连接上了，可以发送数据
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message.c_str(), message.size());
        }
        else
        {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message.c_str(), message.size()));
        }
    }
}

/**
 * 发送数据，应用写得快，内核发送数据慢
 * 需要将数据写入缓冲区，并设置水位回调
 */
void TcpConnection::sendInLoop(const void *data, size_t len)
{
    ssize_t nwrote = 0;      // 实际发送的数据
    ssize_t remaining = len; // 剩余未发送数据
    bool faultError = false;
    if (state_ == kDisconnected)
    {
        LOG_ERROR("TcpConnection::sendInLoop DISCONNECTED give up writing\n");
        return;
    }
    // 没有设置channel写就绪感兴趣且发送缓冲区没有数据，尝试直接发送
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            // 一次发送完了
            if (remaining == 0 && writeCompleteCallback_)
            {
                loop_->runInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else // nwrote < 0,出现错误
        {
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop\n");
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    } // end if(!channel_->isWriting() && outputBuffer_.readableBytes()==0)

    // 还有数据未发送完
    if (!faultError && remaining > 0)
    {
        // 缓冲区的未发送数据长度（可读数据区）
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ // 待发送数据长度大于等于设置的高水位标志
            && oldLen < highWaterMark_           // 且原本的数据长度是小于高水位标志的
            && highWaterMarkCallback_)           // 如果设置了高水位回调
        {
            // 执行高水位回调
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        // 将数据添加到待发送的缓冲区,并设置对channel_的写就绪感兴趣
        // 这样poller会在channel_写就绪时通知channel_执行handleWrite()回调（LT模式，只要写就绪就不停通知）
        outputBuffer_.append((char *)data + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

// 连接建立
void TcpConnection::connectEstablished()
{
    setState(kConnected);
    //弱智能指针指向强智能指针（不增加引用计数）
    channel_->tie(shared_from_this());
    //向poller注册channel的EPOLL_IN事件
    channel_->enableReading();

    //连接建立,调用回调
    connectionCallback_(shared_from_this());
}
// 连接销毁
void TcpConnection::connectDestroyed()
{
    //已经连接了的
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        //把channel感兴趣的事从poller中的epoll_event上del掉
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    //把channel从poller中删除掉
    channel_->remove();
}

//关闭当前连接
void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        //准备关闭，状态置为kDisconnecting后，send()将不能继续发送数据
        //所以，最终都能执行shutdownInLoop()使得fd的写端关闭
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{

    if(!channel_->isWriting())//说明当前outputBuffer_的数据已经发送完成
    {
        //使用::shutdown()关闭sockfd的写端
        socket_->shutdownWrite();
    }
    //如果缓冲区数据没有发送完，因为void TcpConnection::shutdown()中已经将连接状态置为kDisconnecting,那么send()将不能发送数据了
    //当channel可以发送时，handleWrite()会被调用，就可将数据发送完。并且在状态为kDisconnecting时会再一次调用shutdownInLoop()
    //最终fd的写端被关闭了
}
