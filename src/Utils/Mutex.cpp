#include "Utils/Mutex.hpp"

Mutex::Mutex()
{
    int count = 0;
    while(pthread_mutex_init(&mutex, NULL) < 0 && count < 30)
    {
        count++;
    }
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mutex);
}

void Mutex::lock()
{
    pthread_mutex_lock(&mutex);
}

void Mutex::unlock()
{
    pthread_mutex_unlock(&mutex);
}