#pragma once
#include "noncopyable.h"
#include <thread>
#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <unistd.h>
#include "Logger.h"

//
namespace mymuduo
{
class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc, const std::string& name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    bool joined() const { return joined_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }
    static int numCreated() { return numCreated_; }
private:
    void setDefaultName();
    bool started_;
    bool joined_;
    pid_t tid_;
    ThreadFunc func_;
    std::shared_ptr<std::thread> thread_;
    std::string name_;
    static std::atomic_int numCreated_;
};
} // namespace mymudo
