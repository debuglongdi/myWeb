#include "CurrentThread.h"

namespace mymuduo
{
namespace CurrentThread
{   
    //定义
    __thread int t_cachedTid = 0;

    void cachTid()
    {
        if (t_cachedTid == 0)
        {
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
} // namespace CurrentThread
} // namespace mymuduo
