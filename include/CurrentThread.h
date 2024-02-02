#pragma once
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

namespace mymuduo
{
namespace CurrentThread
{   
    //声明t_cachedId，他是每个线程都单独拥有的全局变量
    extern __thread int t_cachedTid;
    void cachTid();
    inline int tid()
    {
        if(__builtin_expect(t_cachedTid == 0, 0))
        {
            cachTid();
        }
        return t_cachedTid;
    }
} // namespace CurrentThread
} // namespace mymuduo
