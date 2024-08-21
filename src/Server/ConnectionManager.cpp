#include "Server/ConnectionManager.hpp"
#include "Utils/Logger.hpp"

#include <iostream>

ConnectionManager::ConnectionManager(int port, SocketClient *serverDaemonClientSocket) : Thread(std::bind(&ConnectionManager::execute, this, std::placeholders::_1), NULL)
{
    this->serverDaemonClientSocket = serverDaemonClientSocket;

    socketServer = SocketServer(port);
}

void *ConnectionManager::execute(void *dummy)
{
    Logger::log("ConnectionManager started");
    while (true) // Thread é parada via stop()
    {
        // Recebe a conexão do cliente, lê a primeira mensagem e encaminha para o ServerDaemon
        SocketServerSession *socket = new SocketServerSession(socketServer.listenAndAccept());

        char* buffer = socket->read();

        if (buffer[0] == CLIENT_CONNECTED_MSG)
        {
            ClientConnectedMsg msg;
            msg.decode(buffer);
            msg.clientSocket = socket;
            char *newBuffer = msg.encode();
            serverDaemonClientSocket->write(newBuffer);
            delete[] newBuffer;
        }
        else if (buffer[0] == NEW_SERVER_MSG)
        {
            NewServerMsg msg;
            msg.decode(buffer);
            msg.address = socket->getClientIP();
            SocketClient *newSocket = new SocketClient(msg.address.c_str(), msg.port);
            newSocket->connect();
            msg.socket = newSocket;
            // msg.port = socket->getClientPort();
            char *newBuffer = msg.encode();
            serverDaemonClientSocket->write(newBuffer);
            delete[] newBuffer;
        }
        else if (buffer[0] == NEXT_IN_RING_MSG)
        {
            NextInRingMsg msg;
            msg.decode(buffer);
            // msg.address = socket->getClientIP();
            // msg.port = socket->getClientPort();
            char *newBuffer = msg.encode();
            serverDaemonClientSocket->write(newBuffer);
            delete[] newBuffer;
        }
        else
        {
            Logger::log("ConnectionManager: Unknown message received: " + std::to_string(+buffer[0]));
        }
        delete[] buffer;
    }
}

void ConnectionManager::stop()
{
    Thread::stop();
    Logger::log("ConnectionManager stopped");
}