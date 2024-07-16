#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/SocketClient.hpp"
#include "Server/UserManager.hpp"
#include <string>
#include <unordered_set>

class ClientManager : public Thread
{
private:
    std::string username;
    bool shouldStop;
    unsigned long long clientId;

    std::unordered_set<std::string> filesBeingEdited;

    SocketServerSession *socket;
    SocketClient *userManagerSocket;
    SocketClient *serverDaemonSocket;

    void *execute(void *dummy);
    void processSyncAllMsg(const char *buffer);
    void processFileDeleteMsg(const char *buffer);
    void processFileWriteMsg(const char *buffer);
    void processEndClientMsg(const char *buffer);
    void processFileUploadMsg(char *buffer);
    void processFileDownloadMsg(const char *buffer);
    void processFileDelCmdMsg(const char *buffer);

public:
    ClientManager(const std::string username, const unsigned long long clientId, SocketServerSession *socket, SocketClient *userManagerSocket, SocketClient *serverDaemonSocket);
    ~ClientManager(){};
};