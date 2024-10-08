#include "Utils/MutexHash.hpp"
#include <string>
#include <iostream>

template <class T>
Mutex* MutexHash<T>::getOrAddMutex(T key)
{
    hashMutex.lock();
    if (mutexes.find(key) == mutexes.end())
    {
        mutexes[key] = new Mutex();
    }
    hashMutex.unlock();
    return mutexes[key];
}

template <class T>
void MutexHash<T>::lock(T key)
{
    mutexes[key]->lock();
}

template <class T>
void MutexHash<T>::unlock(T key)
{
    mutexes[key]->unlock();
}

template class MutexHash<int>;
template class MutexHash<std::string>;
template class MutexHash<char>;