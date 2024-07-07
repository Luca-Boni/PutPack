#pragma once

#include <string>
#include "Utils/SocketClient.hpp"

class Client {
private:
    SocketClient socketClient;
    bool isConnected;

public:
    Client(const std::string& serverAddress, int serverPort);
    ~Client();

    bool connectToServer();
    void disconnectFromServer();
    bool isConnectedToServer() const;

    // Comandos do cliente
    void upload(const std::string& filePath);
    void download(const std::string& fileName);
    void deleteFile(const std::string& fileName);
    void listServerFiles();
    void listClientFiles();
    void synchronize();

private:
    void sendCommand(const std::string& command);
    void receiveResponse();
};
