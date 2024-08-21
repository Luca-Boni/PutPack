#include "Client/ServerReceiver.hpp"
#include "Utils/Protocol.hpp"
#include "Utils/Logger.hpp"

#include <iostream>

ServerReceiver::ServerReceiver(const std::string &serverAddress, int serverPort) : Thread(std::bind(&ServerReceiver::execute, this, std::placeholders::_1), NULL)
{
    serverSocket = new SocketClient(serverAddress.c_str(), serverPort);
    Logger::log("ServerReceiver created for server " + serverAddress + ":" + std::to_string(serverPort));
}

void *ServerReceiver::execute(void *dummy)
{
    if (!connectToServer())
    {
        Logger::log("Server not available at the moment. Stopping program...");
        std::cerr << "Server not available at the moment. Stopping program..." << std::endl;
        exit(0);
    }

    while (clientSocket == NULL && confSocket == NULL)
        ;

    Logger::log("Successfully connected to server");

    while (true)
    {
        char *buffer = serverSocket->read();
        if (buffer == NULL || buffer[0] == SERVER_DEAD)
        {
            continue;
        }

        if (buffer[0] == OK_MSG)
        {
            confSocket->write(buffer);
        }
        else
        {
            if (buffer[0] == FILE_READ_MSG) // Arquivo foi lido no servidor, por isso deve escrever no cliente
            {
                buffer[0] = FILE_WRITE_MSG;
            }
            clientSocket->write(buffer);
        }

        delete[] buffer;
    }

    return NULL;
}

bool ServerReceiver::connectToServer()
{
    serverSocket->connect();
    if (!serverSocket->isConnected())
    {
        return false;
    }

    return true;
}

void ServerReceiver::stop()
{
    Logger::log("ServerReceiver stopped");
    Thread::stop();
}

void ServerReceiver::setServerSocket(SocketClient *serverSocket)
{
    this->serverSocket = serverSocket;
    Logger::log("New server socket is " + serverSocket->getServerIP() + ":" + std::to_string(serverSocket->getServerPort()));
}