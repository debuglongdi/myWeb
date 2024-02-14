#include "TcpServer.h"
#include "Logger.h"
#include "Acceptor.h"
#include "InetAddress.h"

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



TcpServer::TcpServer(EventLoop * loop,
        const InetAddress& listenAddr,
        const std::string& nameArg,
        Option option)
        : loop_(CHECK_NOTNULL(loop))
        , ipPort_(listenAddr.toIpPort())
        , name_(nameArg)
        , acceptor_(new Acceptor(loop, listenAddr, option == kReusePort))
        , threadPool_(new EventLoopThreadPool(loop,name_))
        , connectionCallback_()
        , messageCallback_()
        , nextConnId_(1)
        , started_(0)
{
    /**
     * TcpServer::newConnection
     * 根据轮询算法选择一个subloop
     * 唤醒subloop
     * 把当前connfd 封装成channel丢给subloop
    */
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1,_2));
}

TcpServer::~TcpServer()
{

}

//设置处理事件的线程数目,subLoop
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

//开启服务器监听
void TcpServer::start()
{
    //防止一个TcpServer对象被started多次
    if(started_++ == 0)
    {
        //启动线程池
        threadPool_->start(threadInitCallback_);
        //在loop种执行回调listen()开启监听
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

//not thread safe but in loop
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{

}
//thread safe
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{

}
//not thread safe but in loop
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{

}

