#include "Thread.h"
#include "CurrentThread.h"
#include <semaphore.h>
#include <unistd.h>

namespace mymuduo
{
std::atomic<int> Thread::numCreated_ = 0;
Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(func)
    , name_(name)
{
    setDefaultName();
}
Thread::~Thread()
{
    if(started_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;
    //开启线程
    sem_t sem;
    sem_init(&sem,false, 0);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){

        tid_ = mymuduo::CurrentThread::tid();
        sem_post(&sem);
        //执行线程函数
        func_();
    }));
    //阻塞等待线程创建并获得tid
    sem_wait(&sem);
}
void Thread::join()
{
    joined_ = true;
    thread_->join();
}
void Thread::setDefaultName()
{
    //默认名
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}
} // namespace mymudduo



