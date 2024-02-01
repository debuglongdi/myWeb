#pragma once

namespace mymuduo
{
    /**
     * 私有继承的派生类对象不能执行拷贝构造和赋值
    */
    class noncopyable
    {
    private:
        /* data */
    public:
        noncopyable(const noncopyable &) = delete;
        noncopyable& operator=(const noncopyable &) = delete;

    protected:
        noncopyable(/* args */) = default;
        ~noncopyable() = default;
    };

} // namespace mymuduo
