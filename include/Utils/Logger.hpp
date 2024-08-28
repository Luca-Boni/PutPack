#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/SocketClient.hpp"
#include "Mutex.hpp"
#include <string>
#include <fstream>

class Logger : public Thread
{
private:
    static bool instanceFlag;
    static Mutex logMutex;
    static Logger *logger;
    std::string logFilename;
    SocketServer socketServer;
    SocketServerSession socket;
    SocketClient socketClient;
    std::ofstream logFile;

    void *execute(void *dummy);

public:
    static Logger* getLogger() { return logger; }
    Logger(std::string logFilename = "");

    void stop();
    static void log(const std::string &message);
};