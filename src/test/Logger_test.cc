
#define MYMUDUO_DEBUG

#include "Logger.h"
#include <iostream>
using namespace std;
using namespace mymuduo;



int main(){
    LOG_INFO("TEST INFO");
    LOG_ERROR("TEST ERROR");
    LOG_FATAL("TEST FATAL");
    LOG_DEBUG("TEST DEBUG");
}