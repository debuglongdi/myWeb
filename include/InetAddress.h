#pragma once

#include "copyable.h"
#include <netinet/in.h>
#include <string>

namespace mymuduo
{
namespace net
{
namespace sockets
{
    const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
} // namespace sockets


    /**
     * 封装socket地址
    */
    class InetAddress : public copyable
    {
    private:
        /* data */
        //当前只处理ipv4
        union 
        {
            struct sockaddr_in addr_;
            struct sockaddr_in6 addr6_;
        };
    public:
        /// @brief Constructs an endpoint with given port number.
        ///        Mostly used in TcpServer Listening
        /// @param port 
        /// @param loopbackOnly =false;
        /// @param ipv6 = false;
        explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);

        /// @brief constructor an endpoin with given ip and port
        /// @param ip shoud be like "1.1.1.0"
        /// @param port 
        /// @param ipv6=false
        InetAddress(std::string ip, uint16_t port, bool ipv6 = false);

        /// @brief constructor an endpoint with ginve @ c struct of socketaddr
        ///        mostly used when accepting new connection
        /// @param addr 
        explicit InetAddress(struct sockaddr_in &addr)
        :addr_(addr)
        { }
        explicit InetAddress(struct sockaddr_in6 &addr)
        :addr6_(addr)
        { }
        std::string toIp() const;
        std::string toIpPort() const;
        uint16_t toPort() const;

        const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&addr6_);}
        void setSockAddr(const sockaddr_in addr){ addr_ = addr; }

        ~InetAddress() = default;
    };// end InetAddress
    

} // namespace net
} // namespace mymudo

