#pragma once

#include "Utils/SocketClient.hpp"

class ClientMenu
{
private:
    SocketClient *clientDaemonSocket;

    void help();
    void upload();
    void download();
    void deleteFile();
    void listServerFiles();
    void listClientFiles();
    void synchronize();
    void exit();

public:
    ClientMenu(SocketClient *clientDaemonSocket) : clientDaemonSocket(clientDaemonSocket){};
    ~ClientMenu(){};

    void start();
};