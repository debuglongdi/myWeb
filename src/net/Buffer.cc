#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
using namespace mymuduo;
using namespace mymuduo::net;


/**
 * 从fd上读取数据，写入可写区域，Poller运行在LT模式，边缘触发
 * Buffer缓冲区大小是有限的！但是从fd上读数据时候，却不知道Tcp数据的最终大小
 * 将数据从可读区读取后放到可写区
*/
ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    //栈上的内存空间
    char extrabuf[65536] = {0};
    struct iovec vec[2];
    //buffer缓冲区剩余的可写大大小
    const size_t writeable = writeableBytes();
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writeable;

    //第二块
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    //一次最多读64Kb的数据
    const int iovcnt = (writeable < sizeof extrabuf)?2:1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if(n < 0)
    {
        *saveErrno = errno;
    }
    else if(n <= (ssize_t)writeable)//可写的区间足够存储从fd中读取的数据
    {
        //更新边界
        writeIndex_ +=n;
    }
    else //extrabuf 也写入数据了
    {
        writeIndex_ = buffer_.size();
        //将extrabuf的数据放到buffer_中
        append(extrabuf,n-writeable);
    }
    return n;
}

/**
 * 把buffer中可读区的数据使用fd发送出去
*/
ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(),readableBytes());
    if(n < 0)
    {
        *saveErrno = errno;
    }
    else
    {
        //移动readIndex
        retrive(n);
    }
    return n;
}