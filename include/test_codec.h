#pragma once
#include <functional>
#include <memory>
#include <stdint.h>
#include <endian.h>
#include <string>
#include "Buffer.h"

/**
 * 发送的数据前4个字节存储真实数据的长度为len的int32_t的数
 * 从buffer中读取len长的数据到std::string message中,让messageCallback_处理
 * LengthHeaderCodec相当于一个中间件，实现对TCP字节流数据的分包，它是在应用层的分包。网络层本来就不需要分包
*/
using namespace mymuduo::net;
class LengthHeaderCodec
{
public:
    using  StringMessageCallback = std::function<void(const std::string& message)>;
    LengthHeaderCodec()
    {}

    /// @brief 解码
    /// @param buf 
    void decode(Buffer *buf)
    {
        //buffer中可能有多个包
        while(buf->readableBytes() >= kHeaderLen)
        {
            const void *data = buf->peek();
            //获取前4个字节中的数
            int32_t be32 = *static_cast<const int32_t*>(data);
            //网络序转换为字节序
            const int32_t len = be32toh(be32);

            //长度不合法
            if(len > 65536 || len < 0)
            {
                printf("Invalid length %d\n",(int)len);
                break;
            }
            else if(buf->readableBytes() >= len + kHeaderLen)
            {
                //前4个不读取
                buf->retrive(kHeaderLen);
                std::string message(buf->peek(), len);
                buf->retrive(len);
            }
            else
            {
                break;
            }
        }//end while
    }//end onmessage

    //编码
    //使用协议发送数据，在数据流前面加4个字节，是该数据包的长度，不包含头部
    void encode(const std::string &message)
    {
        //将数据添加到buffer中
        buf.append(message.data(), message.size());
        int32_t len = static_cast<int32_t>(message.size());

        //字节序hton
        int32_t be32 = htobe32(len);
        //在头部加上数据的长度Header
        buf.prepend(&be32, sizeof(be32));
    }
    Buffer buf;
private:
    const static size_t kHeaderLen = sizeof(int32_t);
};