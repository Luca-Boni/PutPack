#include "../../include/Utils/Thread.hpp"
#include <iostream>

using namespace std;

void test(void* args){
    while(true){
        int number = ((int *)args)[0];
        cout << "Hello from thread" << number << endl;
    }
}

int main(){

    Thread t1(test, (void *)new int[1]{1});
    Thread t2(test, (void *)new int[1]{2});

    t1.start();
    t2.start();

    t1.join();
    t2.join();

    return 0;
}