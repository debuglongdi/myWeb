#include "TcpServer.h"
#include "Logger.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "strings.h"
#include "TcpConnection.h"
#include <memory>

using namespace mymuduo;
using namespace mymuduo::net;

namespace
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    // EventLoop* CHECK_NOTNULL(EventLoop* loop)
    // {
    //     if(loop == nullptr)
    //     {
    //         LOG_FATAL("%s:%s:%d mainLoop is null !\n", __FILE__, __FUNCTION__, __LINE__);
    //     }
    //     return loop;
    // }
} // namespace

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const std::string &nameArg,
                     Option option)
    : loop_(CHECK_NOTNULL(loop)), ipPort_(listenAddr.toIpPort()), name_(nameArg), acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)), threadPool_(new EventLoopThreadPool(loop, name_)), connectionCallback_(), messageCallback_(), nextConnId_(1), started_(0)
{
    /**
     * TcpServer::newConnection
     * 根据轮询算法选择一个subloop
     * 唤醒subloop
     * 把当前connfd 封装成channel丢给subloop
     */
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    LOG_INFO("TcpServer::~TcpServer \n");
    //将map管理的连接全部销毁
    for(auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        //销毁了，只剩下conn局部变量，然后离开这个域之后，TcpConnectionPtr就真正销毁了
        item.second.reset();
        //调用TcpConnection::connectDestroyed()销毁每一个连接,从poller中删除连接fd
        conn->getLoop()-> runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn)
        );
    }
}

// 设置处理事件的线程数目,subLoop
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

// 开启服务器监听
void TcpServer::start()
{
    // 防止一个TcpServer对象被started多次
    if (started_++ == 0)
    {
        // 启动线程池
        threadPool_->start(threadInitCallback_);
        // 在loop中执行回调listen()开启监听
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

// not thread safe but only in mainloop
/// @brief 有新的连接，acceptor会执行这个回调
/// @param sockfd
/// @param peerAddr
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    // 轮询算法，选择一个subloop
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s\n", name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    // 获取本地的地址
    sockaddr_in local;
    socklen_t addrlen = sizeof local;
    bzero(&local, sizeof local);

    if (::getsockname(sockfd, (sockaddr *)&local, &addrlen) < 0)
    {
        LOG_ERROR("TcpServer::newConnection can't get local\n");
    }
    InetAddress localAddr(local);

    // 连接成功的sockfd，用TcpConnection连接对象的智能指针来管理
    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                            connName,
                                            sockfd, // 创建channel
                                            localAddr,
                                            peerAddr));
    connections_[connName] = conn;
    //下面的回调是用户设置给TcpSever=>TcpConnection=>channel 然后poller通知channel执行回调
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    
    //设置如何关闭连接的回调
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    //建立连接成功
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));
}
// thread safe
void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}
// not thread safe but in loop
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection [%s]\n",name_.c_str(), conn->name().c_str());
    
    //先将连接的映射从map中移除
    connections_.erase(conn->name());

    //再TcpConnection所属的loop中调用TcpConnection::connectDestroyed将连接移除
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}
