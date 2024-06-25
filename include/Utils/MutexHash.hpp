#pragma once

#include "Utils/Mutex.hpp"
#include <unordered_map>

template <class T>
class MutexHash
{
private:
    std::unordered_map<T, Mutex> mutexes;
    Mutex hashMutex;

public:
    MutexHash() = default;
    ~MutexHash() = default;
    Mutex& getOrAddMutex(T key);
    void lock(T key);
    void unlock(T key);
};
