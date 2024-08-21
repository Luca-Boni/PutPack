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
    bool backup;
    std::string FEAddress;
    int FEPort;
    std::string ownAddress;
    std::string username;
    bool shouldStop;
    unsigned long long clientId;

    Mutex setToPrimaryMutex;

    std::unordered_set<std::string> filesBeingEdited;

    SocketServer backupServerSocket;

    SocketServer serverDaemonSocketServer;
    SocketClient serverDaemonSocketClient;

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
    void processListServerFilesMsg(const char *buffer);

    void processSetToPrimaryMsg(const char *buffer);

    void redirectMsgToBackups(const char *buffer);

public:
    ClientManager(const std::string username, const unsigned long long clientId, SocketServerSession *socket, SocketClient *userManagerSocket, SocketClient *serverDaemonSocket, bool backup, std::string FEAddress, int FEPort, std::string ownAddress);
    ~ClientManager(){};
    SocketClient *getSocketClient() { return &serverDaemonSocketClient; }
    SocketServerSession *getSocketServerSession() { return socket; }

    void setToPrimary();
};