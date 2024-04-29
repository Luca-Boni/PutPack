#pragma once
#include <pthread.h>
#include <signal.h>
#include <functional>

class Thread
{
public:
    Thread();
    ~Thread();
    Thread(std::function<void(void *)> func, void *arg);
    void start();
    void join();

private:
    std::function<void(void *)> threadFunction;
    void *threadArg;
    pthread_t thread;
    struct functionCaller *caller;
};

struct functionCaller
{
    std::function<void(void *)> function;
    void *args;

    static void *callme_static(void *me)
    {
        return static_cast<functionCaller *>(me)->callme();
    }
    void *callme()
    {
        function(args);
        return NULL;
    }

    functionCaller(std::function<void(void *)> c, void *a) : function(c), args(a) {}
};