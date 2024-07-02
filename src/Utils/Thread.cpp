#include "Utils/Thread.hpp"
#include <iostream>
#include <csignal>

Thread::Thread()
{
    std::signal(SIGTERM, [](int signal)
                { exit(0); });
    threadFunction = std::function<void *(void *)>();
    threadArg = NULL;
    isRunning = false;
    caller = new functionCaller(threadFunction, threadArg, &isRunning);
}

Thread::Thread(std::function<void *(void *)> func, void *arg)
{
    std::signal(SIGTERM, [](int signal)
                { exit(0); });
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
    pthread_kill(thread, SIGTERM);
    // pthread_join(thread, NULL);
    isRunning = false;
}

Thread::~Thread()
{
    delete caller;
    stop();
}

bool Thread::running()
{
    return isRunning;
}