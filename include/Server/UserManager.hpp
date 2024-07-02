#pragma once

#include <string>
#include "Utils/Thread.hpp"
#include "Utils/MutexHash.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/FileWriter.hpp"
#include "Utils/FileReader.hpp"
#include "Utils/Hash.hpp"
#include <unordered_map>

class UserManager : public Thread
{
private:
    std::string username;
    bool shouldStop;

    SocketServer socketServer;
    SocketServerSession socket;         // Socket to communicate with ClientManager
    SocketClient socketClient;          // Sent to ServerDaemon and FileReaders to cummunicate with this thread
    int serverDaemonPort;

    MutexHash<std::string> fileMutexes; // Mutexes to protect the files

    std::unordered_map<unsigned long long, FileReader *> fileReaders;
    std::unordered_map<unsigned long long, FileWriter *> fileWriters;
    std::unordered_map<unsigned long long, SocketServerSession *> fileWriterSessions;

    void *execute(void *dummy);
    void processFileWriteMsg(const char *buffer);
    void processFileReadMsg(const char *buffer);

public:
    UserManager(const std::string username, const int serverDaemonPort);
    ~UserManager(){};
    SocketClient *getSocketClient() { return &socketClient; };
    void stopGraciously();
    void stop();
};