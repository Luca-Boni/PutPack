#include "Utils/Thread.hpp"
#include <iostream>

Thread::Thread()
{
    threadFunction = std::function<void(void *)>();
    threadArg = NULL;
    isRunning = false;
    caller = new functionCaller(threadFunction, threadArg);
}

Thread::Thread(std::function<void(void *)> func, void *arg)
{
    threadFunction = func;
    threadArg = arg;
    isRunning = false;
    caller = new functionCaller(threadFunction, threadArg);
}

void Thread::setThreadFunction(std::function<void(void *)> func)
{
    if (!isRunning)
    {
        threadFunction = func;
    }else{
        std::cerr << "Thread is already running" << std::endl;
    }
}

void Thread::setThreadArg(void *arg)
{
    if (!isRunning)
    {
        threadArg = arg;
    }else{
        std::cerr << "Thread is already running" << std::endl;
    }
}

void Thread::start()
{
    isRunning = true;
    pthread_create(&thread, NULL, &functionCaller::callme_static, caller);
}

void Thread::join()
{
    pthread_join(thread, NULL);
    isRunning = false;
}

Thread::~Thread()
{
    delete caller;
    pthread_kill(thread, 0);
    pthread_join(thread, NULL);
}