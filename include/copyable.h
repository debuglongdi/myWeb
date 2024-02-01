#pragma once

namespace mymuduo
{   
    //公有继承后，派生类对象可以被copy,以及赋值
    class copyable
    {
    public:
        /* data */
    protected:
        copyable() = default;
        ~copyable() = default;
    };
    
} // namespace mymuduo

