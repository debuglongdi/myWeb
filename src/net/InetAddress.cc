
#include <string.h>
#include "../include/InetAddress.h"
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <string.h>


// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

using namespace mymuduo;
using namespace mymuduo::net;

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };
//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

//默认参数只需要在声明时给出
//使用一个port,创建server listening socket时使用
InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
    //编译器检查addr_是否是InetAddress的起始位置，不是就在编译阶段抛出错误（错误信息："addr_ offset 0"）
    static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
    static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
    if(ipv6){
        bzero(&addr6_, sizeof addr6_);
        addr6_.sin6_port = htons(port);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
    }else{
        bzero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        in_addr_t ip= loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = htonl(ip);
    }
}


InetAddress::InetAddress(std::string ip, uint16_t port, bool ipv6)
{
    if(ipv6){
        //将ip地址的point表示转换为in6_addr
        const char* str = ip.c_str();
        in6_addr tmp_addr;
        inet_pton(AF_INET6, str, &tmp_addr);

        bzero(&addr6_, sizeof addr6_);
        addr6_.sin6_port = htons(port);
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_addr = tmp_addr;
    }else{
        bzero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    }
}


std::string InetAddress::toIp() const
{
    char buf[64];
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}
std::string InetAddress::toIpPort() const
{
    char buf[64];
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf+end, ":%u", port);
    return buf;
}
uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}