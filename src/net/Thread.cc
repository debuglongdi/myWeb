#include "Thread.h"
#include "CurrentThread.h"
#include <semaphore.h>
#include <unistd.h>


//
namespace mymuduo
{

std::atomic_int Thread::numCreated_(0);


Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
{
    setDefaultName();
#ifdef MYMUDUO_DEBUG
    LOG_DEBUG("Thread::Thread thread_name=%s\n",name_.c_str());
#endif
}
Thread::~Thread()
{
    if(started_ && !joined_)
    {
        //设置线程分离
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
#ifdef MYMUDUO_DEBUG
    LOG_DEBUG("Thread::start() tid=%d start new thread\n",tid_);
#endif
        //专门执行该线程函数
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



