#include <iostream>
#include <map>
#include <unordered_map>
#include <functional>

namespace
{
using Functor = std::function<void()>;
using Callback = std::function<void(Buffer *bufer)>;

class Buffer;


class A 
{
public:
    A(Functor &cb, Buffer* ptr)
    : callback_(cb)
    {
        
    }
    void handle()
    {
        //处理buffer
        callback_(buffer_);
    }
private:
    Buffer *buffer_;
    Callback callback_;
};

class B 
{
public:
private:
    Buffer *buffer_;
    Callback callback_;
};

} // namespace


int main(){
    std::cout<<"hello world\n";
    return 0;
}