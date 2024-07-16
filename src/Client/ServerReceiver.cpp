#include "Client/ServerReceiver.hpp"
#include "Utils/Protocol.hpp"

#include <iostream>

ServerReceiver::ServerReceiver(const std::string &serverAddress, int serverPort) : Thread(std::bind(&ServerReceiver::execute, this, std::placeholders::_1), NULL)
{
    serverSocket = SocketClient(serverAddress.c_str(), serverPort);
}

void *ServerReceiver::execute(void *dummy)
{
    if (!connectToServer())
    {
        std::cerr << "Server not available at the moment. Stopping program..." << std::endl;
        exit(0);
    }

    while (clientSocket == NULL);
    
    while (true)
    {
        char *buffer = serverSocket.read();
        if (buffer == NULL)
        {
            continue;
        }

        if (buffer[0] == FILE_READ_MSG) // Arquivo foi lido no servidor, por isso deve escrever no cliente
            buffer[0] = FILE_WRITE_MSG;
        clientSocket->write(buffer);

        delete[] buffer;
    
    }

    return NULL;
}

bool ServerReceiver::connectToServer()
{
    serverSocket.connect();
    if (!serverSocket.isConnected())
    {
        return false;
    }

    return true;
}