#include <iostream>
#include "InetAddress.h"

using namespace std;
using namespace mymuduo;
using namespace mymuduo::net;

int main(){
    InetAddress addr(8080,true);
    InetAddress addr2("47.98.144.31",7890);
    cout<<"ip:port = "<<addr.toIpPort()<<endl;
    cout<<"ip:port = "<<addr2.toIpPort()<<endl;;
    return 0;
}