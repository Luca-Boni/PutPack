#include "Server/ConnectionManager.hpp"

#include <iostream>

ConnectionManager::ConnectionManager(int port, SocketClient *serverDaemonClientSocket) : Thread(std::bind(&ConnectionManager::execute, this, std::placeholders::_1), NULL)
{
    this->serverDaemonClientSocket = serverDaemonClientSocket;

    socketServer = SocketServer(port);
}

void *ConnectionManager::execute(void *dummy)
{
    while (true) // Thread é parada via stop()
    {
        // Recebe a conexão do cliente, lê a primeira mensagem e encaminha para o ServerDaemon
        SocketServerSession *socket = new SocketServerSession(socketServer.listenAndAccept());

        char* buffer = socket->read();
        ClientConnectedMsg msg;
        msg.decode(buffer);
        msg.clientSocket = socket;
        serverDaemonClientSocket->write(msg.encode());
        delete[] buffer;
    }
}