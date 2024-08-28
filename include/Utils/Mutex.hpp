#pragma once

#include <pthread.h>

class Mutex
{
private:
    pthread_mutex_t mutex;

public:
    Mutex();
    ~Mutex();
    void lock();
    void unlock();
};