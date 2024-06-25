#pragma once

#include <string>
#include "Utils/Thread.hpp"
#include "Utils/MutexHash.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Server/FileHandler.hpp"
#include <vector>

class UserManager : public Thread
{
private:
    std::string username;

    SocketServer socketServer;
    SocketServerSession serverDaemonSocket; // Socket to communicate with ServerDaemon

    // Mutexes to protect the files
    MutexHash<std::string> filesMutex;
    std::vector<FileHandler> fileHandlers;
    
    
};