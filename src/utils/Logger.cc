#include "Logger.h"
#include "Timestamp.h"
#include <iostream>

using namespace mymuduo;

//使用静态局部变量，线程安全
Logger& Logger::getInstance(){
    static Logger logger;
    return logger;
}
//设置日志级别
void Logger::setLogLevel(int level){
    loglevel_ = level;
}
//写日志
void Logger::log(const std::string msg){
    switch (loglevel_)
    {
    case INFO:
        std::cout<< "[INFO]";
        break;
    case ERROR:
        std::cout<< "[ERRO]";
        break;
    case FATAL:
        std::cout<< "[FATAL]";
        break;
    case DEBUG:
        std::cout<< "[DEBUG]";
        break;
    default:
        break;
    }
    //打印时间和msg
    //Timestamp::now().toString()
    std::cout<< Timestamp::now().toString()<<" "<<msg<<std::endl;
}




