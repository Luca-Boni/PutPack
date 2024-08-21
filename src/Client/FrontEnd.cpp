#include "Client/FrontEnd.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Protocol.hpp"

FrontEnd::FrontEnd(SocketClient *clientDaemon) : Thread(std::bind(&FrontEnd::execute, this, std::placeholders::_1), NULL)
{
    this->clientDaemon = clientDaemon;
    socketServer = SocketServer();
    Logger::log("FrontEnd created on port " + std::to_string(socketServer.getPort()));
}

void *FrontEnd::execute(void *dummy)
{
    while (true)
    {
        SocketServerSession *socket = new SocketServerSession(socketServer.listenAndAccept());
        Logger::log("FrontEnd received connection");
        char *buffer = socket->read();
        NewServerMsg msg;
        msg.decode(buffer);
        msg.socketSession = socket;
        char *newBuffer = msg.encode();
        clientDaemon->write(newBuffer);
        delete[] newBuffer;
        delete[] buffer;
    }
    return NULL;
}