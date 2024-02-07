#include <stdlib.h>
#include "Poller.h"
#include "EpollPoller.h"
// #include "../net/Epoller.h"

using namespace mymuduo;
using namespace mymuduo::net;

Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;
    }
    else
    {
        //TODO new Epoller(loop);
        return new EpollPoller(loop);
    }
}