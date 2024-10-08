#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketClient.hpp"

#include <string>

class ServerReceiver : public Thread
{
private:
    SocketClient serverSocket; // Receives messages from the server
    SocketClient *clientSocket; // Sends messages to ClientDaemon

    void *execute(void *dummy);

    bool connectToServer();

public:
    ServerReceiver(const std::string &serverAddress, int serverPort);
    ServerReceiver(){};
    ~ServerReceiver(){};
    
    SocketClient *getServerSocket() { return &serverSocket; };
    void setClientSocket(SocketClient *clientSocket) { this->clientSocket = clientSocket; };

    void stop();
};