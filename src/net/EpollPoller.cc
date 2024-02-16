#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <CurrentThread.h>

using namespace mymuduo;
using namespace mymuduo::net;

namespace
{
    // channel没加入poller中
    const int kNew = -1;
    // channel没加入了poller中
    const int kAdded = 1;
    // channel从poller中删除
    const int kDeleted = 2;
}

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        // 创建epoll失败
        LOG_FATAL("EpollPoller::EpollPoller epoll_create1\n");
    }
}

EpollPoller::~EpollPoller()
{
    close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_INFO("tid=%d EpollPoller::poll(), func=%s\n",CurrentThread::tid(), __FUNCTION__);
    //阻塞等待感兴趣的事件发生或者超时返回
    int numEvents = epoll_wait(epollfd_,
                                &*events_.begin(),
                                static_cast<int>(events_.size()),
                                timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0)
    {
        LOG_INFO("envets happend\n");
        fillActiveChannels(numEvents, activeChannels);

        if(numEvents == (int)events_.size())
        {
            //可能发生的事件较多，给数组vector扩容
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_INFO("nothing happend\n");
    }
    else
    {
        //如果不是系统终端引起的错误，就LOG_ERROR上报错误
        if(saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EpollPoller::poll err\n");
        }
    }
    return now;
}

void EpollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("EpollPoller::updateChannel(), fd=%d events=%d index=%d \n", channel->fd(), channel->events(), channel->index());
    if (index == kNew || index == kDeleted)
    {
        // 添加不存在epoll中的fd
        if (index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        // 更新存在epoll中的fd
        // int fd = channel->fd();
        // 如果对所有事件都不感兴趣，从epollfd上删除之（不从channels_上移除）
        if (channel->isNonEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            // 更新
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel *channel)
{
    LOG_INFO("EpollPoller::removeChannel, fd=%d \n", channel->fd());
    int fd = channel->fd();
    int index = channel->index();
    if (channels_.find(fd) == channels_.end())
    {
        LOG_ERROR("can't delete a channel which is't in poller\n");
    }
    // 从channelMap中移除
    channels_.erase(fd);
    // 已经加入到epoll中，就要从其中移除
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

/// @brief 向activeChannels中插入numEvents个已经就绪的事件
/// @param numEvents 
/// @param activeChannels 
void EpollPoller::fillActiveChannels(int numEvents,
                                     ChannelList *activeChannels) const
{
    for(int i = 0; i < numEvents; i++)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollPoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    int fd = channel->fd();
    // int index = channel->index();
    // 执行epoll_ctl op
    event.events = channel->events();
    event.data.ptr = channel;
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error \n");
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error \n");
        }
    }
}