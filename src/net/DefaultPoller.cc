#include <stdlib.h>
#include "../include/Poller.h"
// #include "../net/Epoller.h"

using namespace mymuduo;
using namespace mymuduo::net;

Poller* newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;
    }
    else
    {
        //TODO new Epoller(loop);
        return nullptr;
    }

}