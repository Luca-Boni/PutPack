#pragma once

#include "Utils/Thread.hpp"
#include "Server/ServerDaemon.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/SocketClient.hpp"

class ConnectionManager : public Thread
{
private:
    SocketServer socketServer;
    SocketClient *serverDaemonClientSocket;

    void *execute(void *dummy);

public:
    ConnectionManager(){};
    ConnectionManager(int port, SocketClient *serverDaemonClientSocket);
    ~ConnectionManager(){};
    int getPort() { return socketServer.getPort(); };
};