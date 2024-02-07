#pragma once

#include"noncopyable.h"
#include<string>
//LOG_INFO("%s %d", arg1, arg2)
#define LOG_INFO(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::getInstance(); \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024,logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0) \

//LOG_ERROR("%s %d", arg1, arg2)
#define LOG_ERROR(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::getInstance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024,logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0) \

//LOG_FATAL("%s %d", arg1, arg2)
#define LOG_FATAL(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::getInstance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024,logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0) \


#ifdef MYMUDUO_DEBUG

//LOG_DEBUG("%s %d", arg1, arg2)
#define LOG_DEBUG(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::getInstance(); \
        logger.setLogLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024,logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0) \

#endif

namespace mymuduo
{
    enum LogLevl
    {
        INFO, //普通信息
        ERROR, //错误信息
        FATAL, //core信息
        DEBUG, //调试信息
    };

    //sigleton模式
    class Logger : noncopyable
    {
    public:
        //获取日志实例
        static Logger& getInstance();
        //设置日志级别
        void setLogLevel(int level);
        //写日志
        void log(const std::string msg);
    private:
        Logger() = default;
        int loglevel_;
    };
}
