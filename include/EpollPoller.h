#pragma once
#include "Poller.h"
#include <sys/epoll.h>

// struct epoll_event{
//     uint32_t events;
//     epoll_data data;
// }

// typedef union epoll_data {
//     void *ptr;/* pointer to user-define data*
//     int *fd;/* file descriptor*/
//     uint32_t u32;/*32-bit integer*/
//     uint64_t u64;/*64-bit integer*/
// }epoll_data;


namespace mymuduo
{
namespace net
{
    class EpollPoller : public Poller
    {
    public:
        EpollPoller(EventLoop *loop);
        ~EpollPoller() override;

        Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
        void updateChannel(Channel* channel) override;
        void removeChannel(Channel* channel) override;
    private:
        //初始化events_的大小
        static const int kInitEventListSize = 16;
        static const char* operationToString(int op);
        void fillActiveChannels(int numEvents,
                                ChannelList* activeChannels) const;
        void update(int operation, Channel* channel);
        using EventList = std::vector<epoll_event>;
        int epollfd_;
        //已经发生的事件的集合
        EventList events_;
    };
    
    
} // namespace net
} // namespace mymuduo
