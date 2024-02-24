#include "TcpClient.h"
#include "Logger.h"
#include "Connector.h"
#include "SockOps.h"
#include <strings.h>

using namespace mymuduo;
using namespace mymuduo::net;

namespace mymuduo
{
namespace net
{
namespace detail
{

    // not thread safe but in loop;
    void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn)
    {
        loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }

    void removeConnector(const ConnectorPtr &connector)
    {
        // connector->
        //   connector.reset();
    }

} // namespace detail
} // namespace net
} // namespace mymuduo
using std::placeholders::_1;
using std::placeholders::_2;
TcpClient::TcpClient(EventLoop *loop,
                     const InetAddress &serverAddr,
                     const std::string &nameArg)
    : loop_(CHECK_NOTNULL(loop)),
      connector_(new Connector(loop_, serverAddr)),
      name_(nameArg),
      connectionCallback_(),
      messageCallback_(),
      retry_(false),
      connect_(true),
      nextConnId_(1),
      mutex_()
{
    connector_->setNewConnectionCallbac(
        std::bind(&TcpClient::newConnection, this, _1));
    LOG_INFO("TcpClient::TcpClient\n");
}
TcpClient::~TcpClient()
{
    LOG_INFO("TcpClient::~TcpClient\n");
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    // 有连接
    if (conn)
    {
        CloseCallback cb = std::bind(&detail::removeConnection, loop_, conn);
        loop_->runInLoop(
            std::bind(&TcpConnection::setCloseCallback, conn, cb));
        // 连接只有一个线程在用，就是当前线程
        if (unique)
        {
            // 关闭连接
            conn->forceClose();
        }
    }
    else
    {
        connector_->stop();
        // TODO :use runAfter
        loop_->runInLoop(std::bind(&detail::removeConnector, connector_));
    }
}

void TcpClient::connect()
{
    // FIXME: check state
    LOG_INFO("TcpClient::connect[] - connecting to \n");//, connector_->serverAddress().toIpPort());
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_ = false;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (connection_)
        {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop()
{
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
    // 获取sockfd 本地地址
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen1 = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, (sockaddr *)&localaddr, &addrlen1) < 0)
    {
        LOG_ERROR("sockets::getLocalAddr");
    }
    /// 获取对端地址
    struct sockaddr_in peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen2 = sizeof(peeraddr);
    if (::getpeername(sockfd, (sockaddr *)&peeraddr, &addrlen2) < 0)
    {
        LOG_ERROR("sockets::getPeerAddr");
    }

    InetAddress localAddr(localaddr);
    InetAddress peerAddr(peeraddr);
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    LOG_INFO("local=%s perraddr=%s\n",localAddr.toIpPort().c_str(), peerAddr.toIpPort().c_str());
    TcpConnectionPtr conn(new TcpConnection(loop_,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, _1)); // FIXME: unsafe
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
  {
    std::unique_lock<std::mutex> lock(mutex_);
    // assert(connection_ == conn);
    connection_.reset();
  }

  loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  if (retry_ && connect_)
  {
    LOG_INFO("TcpClient::connect[] - Reconnecting to\n");//connector_->serverAddress().toIpPort());
    connector_->restart();
  }
}

TcpConnectionPtr TcpClient::connection() const
{
    // std::unique_lock<std::mutex> lock(mutex_);
    {
        //FIX ME
        // std::unique_lock<std::mutex> lock(mutex_);
        return connection_;
    }
}