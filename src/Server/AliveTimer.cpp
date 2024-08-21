#include "Server/AliveTimer.hpp"
#include "Utils/Logger.hpp"

AliveTimer::AliveTimer(int seconds, std::function<void*(void*)> callback) : Thread(std::bind(&AliveTimer::execute, this, std::placeholders::_1), NULL),
                                                                                                                shouldStop(false),
                                                                                                                seconds(seconds)
{
    this->callback = callback;
    Logger::log("AliveTimer created");
}

void *AliveTimer::execute(void *dummy)
{
    Logger::log("AliveTimer started");
    while (!shouldStop)
    {
        sleep(seconds);
        if (callback)
            callback(NULL);
    }
    Logger::log("AliveTimer stopped");
    return NULL;
}

void AliveTimer::stopGraciously()
{
    shouldStop = true;
}