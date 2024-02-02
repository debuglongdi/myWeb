#include <iostream>

int main(){
    std::cout<<"hello world\n";
    return 0;
}
// 那如果有四个类：EventLoop, Channel, Poller, EpollPoller
// 如下：
class Channel{
private:
    EventLoop* loop_;
};
class Poller{
public:
    virtual void poll() = 0;
protected:
    unordered_map<int, Channel*> channels_;
private:
    EventLoop* ownerLoop_;
};

//EpollPoller 是Poller的派生类
class EpollPoller : public Poller{

};

class EventLoop{
private:
    Poller* poller_;
};