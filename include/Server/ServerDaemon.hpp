#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/SocketClient.hpp"
#include "Server/ClientManager.hpp"
#include "Server/UserManager.hpp"
#include "Utils/Logger.hpp"
#include "Server/AliveTimer.hpp"
#include "Server/BackupClient.hpp"

#include <string>
#include <vector>
#include <unordered_map>

struct BackupInfo
{
    int id;
    std::string address;
    int port;
    SocketClient *socket;

    BackupInfo(int id, std::string address, int port, SocketClient *socket) : id(id), address(address), port(port), socket(socket) {}
    BackupInfo() : id(0), address(""), port(0), socket(NULL) {}
    bool operator>(const BackupInfo& other) { return this->id > other.id; }
};

class ServerDaemon : public Thread
{
private:
    bool shouldStop;
    bool backup;
    std::string primaryAddress;
    int primaryPort;
    int id;
    std::string ownAddress;
    int ownPort;
    std::unordered_map<std::string, UserManager *> userManagers;
    std::unordered_map<std::string, std::unordered_map<unsigned long long, ClientManager *>> clientManagers;

    SocketServer socketServer;
    SocketServerSession socket; // Socket to read from ConnectionManager and ClientManagers
    SocketClient socketClient;  // Socket to which ConnectionManager, UserManagers and ClientManagers write

    SocketClient primaryServerSocket;

    std::vector<BackupInfo> backupServers;

    SocketClient nextServer;

    BackupClient* primaryServer;
    BackupClient* prevInRing;
    BackupClient* nextInRing;

    AliveTimer aliveTimer;

    void *execute(void *dummy);
    void processClientConnectedMsg(const char *buffer);
    void processClientDisconnectedMsg(const char *buffer);
    void processNewServerMsg(const char *buffer);
    void processNextInRingMsg(const char *buffer);

    void processMsgFromPrimary(const char *buffer);
    void processSetToPrimaryMsg(const char *buffer);

    void *aliveTimerCallback(void *dummy);

    void connectToPrimary();


public:
    ServerDaemon(int id = 0, bool backup = false, std::string primaryAddress = "");
    ~ServerDaemon() {};
    void stopGraciously();
    SocketClient *getSocketClient() { return &socketClient; };
    void setPort(int port) { ownPort = port; };

    void setToPrimary();
};