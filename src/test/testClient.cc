#include <mymuduo/TcpClient.h>
#include <mymuduo/TcpConnection.h>
#include <mymuduo/Logger.h>
#include <mymuduo/Thread.h>
#include <mymuduo/EventLoopThread.h>
#include <string>

using namespace mymuduo;
using namespace mymuduo::net;
using namespace std::placeholders;

class ChatClient : noncopyable
{
public:
    ChatClient(EventLoop* loop,
              const  InetAddress &serverAddr)
              :loop_(loop),
              client_(loop, serverAddr,"chatClient")
    {
        client_.setConnectionCallback(
            std::bind(&ChatClient::onConnection, this, _1)
        );

        client_.setMessageCallback(
            std::bind(&ChatClient::onMessage,this,_1, _2, _3)
        );
        client_.enableRetry();
    }
    ~ChatClient()
    {

    }
    void connect()
    {
        client_.connect();
    }

    void disconnect()
    {
        client_.disconnect();
    }

    void write(Buffer* buf)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (connection_)
        {
            //使用连接发送buf
            connection_->send(buf);
        }
    }
private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO("connected to [%s] \n",conn->peerAddress().toIpPort().c_str());
        std::unique_lock<std::mutex> lock(mutex_);
        if (conn->connected())
        {
            std::cout<<"success connected\n";
            connection_ = conn;
        }
        else
        {
            connection_.reset();
        }
    }
    void onMessage(const TcpConnectionPtr& conn,
                Buffer *message,
                Timestamp)
    {
        // std::string msg = message->retriveAllAsString();
        std::string msg = message->retriveAllAsString();
        std::cout<<"receive:"<<msg<<std::endl;
    }
    EventLoop* loop_;
    TcpClient client_;
    std::mutex mutex_;
    TcpConnectionPtr connection_;
};

int main(int argc, char * argv[])
{
    if(1)
    {
        EventLoopThread loopThread;
        //端口号
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        //test
        port = 4090;
        std::string ip = "127.0.0.1";
        InetAddress serverAddr(ip, port);

        // InetAddress serverAddr(argv[1], port);
        //在新线程中执行loop
        ChatClient client(loopThread.startLoop(), serverAddr);
        //建立连接
        client.connect();
        std::string line;
        Buffer* buf = new Buffer();
        while (std::getline(std::cin, line))
        {
            //检查内存是否被占用太多
            printf("Buffer size=%lu\n",buf->bufferSize());
            //每次使用前直接将其下标指向正确的地方
            buf->retrieveAll();
            buf->append(&*line.begin(),line.size());
            client.write(buf);
            line.clear();
            
        }
        client.disconnect();
        // CurrentThread::sleepUsec(1000*1000);  // wait for disconnect, see ace/logging/client.cc
    }
    else
    {
        printf("Usage: %s host_ip port\n", argv[0]);
    }
    return 0;
}