#include "Utils/Thread.hpp"
#include <iostream>
#include <csignal>

Thread::Thread()
{
    threadFunction = std::function<void *(void *)>();
    threadArg = NULL;
    isRunning = false;
    caller = new functionCaller(threadFunction, threadArg, &isRunning);
}

Thread::Thread(std::function<void *(void *)> func, void *arg)
{
    threadFunction = func;
    threadArg = arg;
    isRunning = false;
    caller = new functionCaller(threadFunction, threadArg, &isRunning);
}

void Thread::setThreadFunction(std::function<void *(void *)> func)
{
    if (!isRunning)
    {
        threadFunction = func;
        caller->function = threadFunction;
    }
    else
    {
        std::cerr << "Thread is already running" << std::endl;
    }
}

void Thread::setThreadArg(void *arg)
{
    if (!isRunning)
    {
        threadArg = arg;
        caller->args = threadArg;
    }
    else
    {
        std::cerr << "Thread is already running" << std::endl;
    }
}

void Thread::start()
{
    pthread_create(&thread, NULL, &functionCaller::callme_static, caller);
}

void Thread::join()
{
    pthread_join(thread, NULL);
}

void Thread::stop()
{
    if (isRunning)
        pthread_cancel(thread);
    isRunning = false;
}

Thread::~Thread()
{
    if (caller != NULL)
        delete caller;
    stop();
}

bool Thread::running()
{
    return isRunning;
}