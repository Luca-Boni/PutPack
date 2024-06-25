#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/SocketClient.hpp"

class ServerDaemon : public Thread
{
private:
    SocketServer socketServer;
    
};