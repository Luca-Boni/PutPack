#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/SocketClient.hpp"

#include <string>

class FrontEnd : public Thread
{
private:
    SocketServer socketServer;
    SocketClient *clientDaemon;

    void *execute(void *dummy);

public:
    FrontEnd(SocketClient *clientDaemon);
    FrontEnd() {}
    ~FrontEnd() {}

    int getPort() { return socketServer.getPort(); }
};