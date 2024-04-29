#include "../../include/Utils/Thread.hpp"

Thread::Thread(){}

Thread::Thread(std::function<void(void*)> func, void *arg){
    caller = new functionCaller(func, arg);
}

void Thread::start(){
    pthread_create(&thread, NULL, &functionCaller::callme_static, caller);
}

void Thread::join(){
    pthread_join(thread, NULL);
}

Thread::~Thread(){
    delete caller;
    pthread_kill(thread, 0);
    pthread_join(thread, NULL);
}