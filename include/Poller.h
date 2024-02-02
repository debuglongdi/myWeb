#pragma once
#include <unordered_map>
#include <vector>
#include "EventLoop.h"
#include "Timestamp.h"

namespace mymuduo
{
namespace net
{
class Channel;
/**
 * Base class for I/O Multiplexing
*/
class Poller : noncopyable
{
public:
    //装Channel的vector  
    using ChannelList = std::vector<Channel*>;

    explicit Poller(EventLoop * loop);

    virtual ~Poller() = default;

    /// @brief 
    /// @param timeoutMs 超时时间
    /// @param activeChannels Poller监听的所有channel的vector
    /// @return 
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    /// @brief 改变 interested I/O events
    /// must called in the loop thread
    /// @param channel 
    virtual void updateChannel(Channel* channel) = 0;

    /// @brief remove the channel, when it destructs
    /// must called in the loop thread
    /// @param channel 
    virtual void removeChannel(Channel* channel) = 0;

    /// @brief 创建默认的poller派生类对象，并将其赋值给poller指针
    /// @param loop 
    /// @return 
    static Poller* newDefaultPoller(EventLoop *loop);

    virtual bool hasChannel(Channel *channel) const;

protected:
    /// @brief map=key:sockfd, value:channel*
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;
};
    
} // namespace net
} // namespace mymuduo
