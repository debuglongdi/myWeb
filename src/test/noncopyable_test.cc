
#include "../include/noncopyable.h"
#include <iostream>
using namespace std;
using namespace mymuduo;

class A : noncopyable{
private:
    int x;
public:
    A():x(0){};
    A(const A& other){
        *this = other;
    }
    A& operator=(const A& other){
        if(this != &other)
        this->x = other.x;
        return *this;
    }
};

int main(){
    //自己添加了copy constructor和 =重载后，这一特性被打破了
    A a,b;
    A c(a); //无法copy constructor
    b = a; //无法赋值 assignment
    cout<<"不能copy constructor && 对象不能赋值"<<endl;
    return 0;
}