#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>
#include <string>
#include <mymuduo/Thread.h>
// #include <mymuduo/codec.h>

using namespace mymuduo;
using namespace mymuduo::net;
using namespace std::placeholders;

#ifdef MYMUDUO_DEBUG
    LOG_DEBUG("numThreads_=%d\n",numThreads_);
#endif

class EchoServer
{
public:
    EchoServer(EventLoop *loop,
                const InetAddress &addr,
                const std::string &name)
                : loop_(loop),
                server_(loop,addr,name)
    {
        //注册回调函数
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection,this,std::placeholders::_1)
        );
        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage,this, _1,_2,_3)//先执行A,在执行B
        );
        //设置合适的线程数量
        server_.setThreadNum(3);
    }

    void start()
    {
        server_.start();
    }
private:
    //设置建立连接或者断开的回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected())
        {
            LOG_INFO("an new connection:[%s] is connected\n",conn->peerAddress().toIpPort().c_str());
        }
        else
        {
             LOG_INFO("connection:[%s] down\n",conn->peerAddress().toIpPort().c_str());
        }
    }
    //可读写事件的回调
    void onMessage(const TcpConnectionPtr &conn,
                Buffer *msg,
                Timestamp time)
    {
        //echo server
        conn->send(msg);
    }
    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(4090);
    //Acceptor 创建一个non-blocking listenfd (socket() bind() listen())
    EchoServer myserver(&loop,addr,"echoServer-01");
    
    //使用TcpServer listenfd=>acceptchannel=>mainloop=>

    LOG_INFO("start Tcpsever and thread if have\n");
    // 阻塞在这里了,这里不应该阻塞的
    myserver.start();

    LOG_INFO("start mainloop\n");
    //启动mainloop的底层poller
    loop.loop();
    return 0;
}