#pragma once

#include "Utils/Thread.hpp"

class AliveTimer : public Thread
{
private:
    bool shouldStop;
    int seconds;

    std::function<void*(void*)> callback;

    void *execute(void *dummy);

public:
    AliveTimer(int seconds = 0, std::function<void*(void*)> callback = NULL);
    ~AliveTimer(){};
    void stopGraciously();
};