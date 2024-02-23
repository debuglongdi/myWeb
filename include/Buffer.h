#pragma once
#include "copyable.h"
#include <vector>
// #include <stdio.h>
#include <string>
#include <algorithm>

namespace mymuduo
{
namespace net
{
/**
 * 网络库底层的缓冲区
*/
class Buffer : public copyable
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    
    explicit Buffer(size_t initialSize = kInitialSize)
                    : buffer_(kCheapPrepend + initialSize )
                    , readIndex_(kCheapPrepend)
                    , writeIndex_(kCheapPrepend)
    {}
    //可读的数据大小
    size_t readableBytes() const { return writeIndex_ - readIndex_; }
    //可写的区间的大小
    size_t writeableBytes() const { return buffer_.size() - writeIndex_; }
    //缓冲区的大小，默认用8bytes来表示，可以表示2^64 字节的大小的区间
    size_t prependableBytes() const { return readIndex_; }
    //返回缓冲区中可读数据的起始地址
    const char* peek() const
    {
        return begin() + readIndex_;
    }
    //onMessage string <- Buffer
    void retrive(size_t len)
    {
        //读取的数据小于可读数据
        if(len < readableBytes())
        {
            //应用只只读了长度为len的数据还有数据没读完
            readIndex_ +=len;
        }
        else // len == readableBytes()
        {
            retrieveAll();
        }
    }
    void retrieveAll()
    {
        //readIndex_ = writeIndex_ 复位说明缓冲区空闲了可一用来写
        readIndex_ = writeIndex_ = kCheapPrepend;
    }

    /// 把onMessage 上报的Buffer数据,转换为string对象返回
    std::string retriveAllAsString()
    {
        return retriveAsString(readableBytes());
    }
    /// @brief 
    /// @param len 
    /// @return 
    std::string retriveAsString(size_t len)
    {
        //构造数据,将长度为len的数据构造string 
        std::string result(peek(), len);
        //数据读完后，重新整理缓冲区
        retrive(len);
        return result;
    }

    /// @brief 要写的数据长度为len,根据空间剩余确定是扩容还是移动数据（和内存碎片一样）
    /// @param len 
    void ensureWriteableBytes(size_t len)
    {
        if(writeableBytes() < len)
        {
            makeSpace(len);
        }
    }

    /// @brief 将数据添加到可写的缓冲区
    /// @param data 
    /// @param len 
    void append(const char* data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data+len, beginWrite());
        writeIndex_ +=len;
    }
    char* beginWrite()
    {
        return begin() + writeIndex_;
    }
    const char* beginWrite() const
    {
        return begin() + writeIndex_;
    }
    /// @brief 
    /// @param fd 
    /// @param saveErrno 
    /// @return 
    ssize_t readFd(int fd, int* saveErrno);

    ssize_t writeFd(int fd, int* saveErrno);

private:
    //获取底层的地址
    char* begin()
    { return &*buffer_.begin(); }
    const char* begin() const
    { return &*buffer_.begin(); }

    //扩容容器vector
    void makeSpace(size_t len)
    {
        //读取后剩余的空间不够+可写的区间都不够
        if(writeableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writeIndex_ + len);
        }
        else //空间够，但是需要移动可读数据
        {
            //移动数据 将可读数据向前移动到kCheapPrepend
            size_t readable = readableBytes();
            std::copy(begin() + readIndex_,
                      begin() + writeIndex_,
                      begin() + kCheapPrepend);
            //更新数据偏移
            readIndex_ = kCheapPrepend;
            writeIndex_ = readIndex_ + readable;
        }
    }
    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;
};
} // namespace net

} // namespace mymuduo