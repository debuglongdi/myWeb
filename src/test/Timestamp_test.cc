#include "../include/Timestamp.h"
#include <iostream>
using namespace mymuduo;

int main(){
    Timestamp t(time(NULL));
    std::cout<<"time of now is: "<<Timestamp::now().toString()<<std::endl;
    return 0;
}