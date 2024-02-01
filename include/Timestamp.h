#pragma once
#include"../include/copyable.h"
#include <iostream>

namespace mymuduo
{
    class Timestamp : public copyable
    {
    private:
        /* data */
        int64_t microSecondSinceEpoch_;

    public:
        ///
        ///constructor with invalid Timestamp
        Timestamp()
        :microSecondSinceEpoch_(0)
        {
        }
        /// @brief /whoami
        /// @param microSecondSinceEpochArg 
        explicit Timestamp(int64_t microSecondSinceEpochArg)
        :microSecondSinceEpoch_(microSecondSinceEpochArg)
        {
        }
        ~Timestamp() = default;

        /// @brief 
        /// @return 
        static Timestamp now();
        
        /// @brief 
        /// @return 
        std::string toString() const;
    };    

} // namespace mymuduo
