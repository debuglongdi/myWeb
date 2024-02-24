#include "test_codec.h"
#include <string>
#include <iostream>

using namespace std;

int main()
{
    string test1;
    while(cin>>test1)
    {
        LengthHeaderCodec cd;
        printf("R_idx=%d,W_idx=%d\n",(int)cd.buf.readIDX(), (int)cd.buf.writeIDX());
        cout<<"readbale="<<cd.buf.readableBytes()<<endl;
        
        cout<<"encode:"<<test1<<endl;
        cd.encode(test1);
        cout<<"readbale="<<cd.buf.readableBytes()<<endl;
        printf("R_idx=%d,W_idx=%d\n",(int)cd.buf.readIDX(), (int)cd.buf.writeIDX());


        const void *data = cd.buf.peek();
        //获取前4个字节中的数
        int32_t be32 = *static_cast<const int32_t*>(data);
        //网络序转换为字节序
        const int32_t len = be32toh(be32);
        printf("get the Headr of the buffer to get len=%d\n",len);




        std::string st(cd.buf.peek(), 13);
        cout<<"peek="<<st<<endl;
        cout<<"result:"<<cd.buf.retriveAllAsString()<<endl;
    }
    
}