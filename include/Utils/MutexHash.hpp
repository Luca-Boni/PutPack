#pragma once

#include "Utils/Mutex.hpp"
#include <unordered_map>

template <class T>
class MutexHash
{
private:
    std::unordered_map<T, Mutex*> mutexes;
    Mutex hashMutex;

public:
    MutexHash(){hashMutex = Mutex(); mutexes = std::unordered_map<T, Mutex*>();};
    ~MutexHash(){for (auto it = mutexes.begin(); it != mutexes.end(); it++) delete it->second;};
    Mutex* getOrAddMutex(T key);
    void lock(T key);
    void unlock(T key);
};