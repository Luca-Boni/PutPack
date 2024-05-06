#include "Utils/Thread.hpp"
#include <iostream>
#include <csignal>

Thread::Thread()
{
    std::signal(SIGTERM, [](int signal) { exit(0); });
    threadFunction = std::function<void(void *)>();
    threadArg = NULL;
    isRunning = false;
    caller = new functionCaller(threadFunction, threadArg);
}

Thread::Thread(std::function<void(void *)> func, void *arg)
{
    std::signal(SIGTERM, [](int signal) { exit(0); });
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

void Thread::stop()
{
    pthread_kill(thread, SIGTERM);
    // pthread_join(thread, NULL);
    isRunning = false;
}

Thread::~Thread()
{
    delete caller;
    stop();
}