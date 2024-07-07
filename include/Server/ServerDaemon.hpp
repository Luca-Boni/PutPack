#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/SocketClient.hpp"
#include "Server/ClientManager.hpp"
#include "Server/UserManager.hpp"

#include <string>
#include <vector>
#include <unordered_map>

class ServerDaemon : public Thread
{
private:
    bool shouldStop;
    std::unordered_map<std::string, UserManager *> userManagers;
    std::unordered_map<std::string, std::unordered_map<unsigned long long, ClientManager *>> clientManagers;

    SocketServer socketServer;
    SocketServerSession socket; // Socket to read from ConnectionManager and ClientManagers
    SocketClient socketClient;  // Socket to which ConnectionManager and ClientManagers write

    void *execute(void *dummy);
    void processClientConnectedMsg(const char *buffer);
    void processNewClientMsg(const char *buffer);
    void processClientDisconnectedMsg(const char *buffer);

public:
    ServerDaemon();
    ~ServerDaemon(){};
    void stopGraciously();
    SocketClient *getSocketClient() { return &socketClient; };
};