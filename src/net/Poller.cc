#include "../include/Poller.h"
#include "../include/EventLoop.h"
#include "../include/Channel.h"

using namespace mymuduo;
using namespace mymuduo::net;

Poller::Poller(EventLoop *loop)
    : ownerLoop_(loop)
{ }

bool Poller::hasChannel(Channel *channel) const
{
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return (it != channels_.end() && it->second == channel);
}