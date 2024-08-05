#pragma once

#include "Utils/Mutex.hpp"
#include "Utils/SocketServer.hpp"

#include <arpa/inet.h>
#include <string>
#include <tuple>

class SocketServerSession
{
private:
    socket_t reader_fd;
    Mutex writeMutex;
    Mutex readMutex;
    struct sockaddr_in clientAddress;

public:
    SocketServerSession(){};
    SocketServerSession(std::tuple<socket_t, struct sockaddr_in> sessionInfo);
    ~SocketServerSession(){};
    void write(const char* buffer);
    char* read();
    void close();

    std::string getClientIP() { return std::string(inet_ntoa(clientAddress.sin_addr)); }
    int getClientPort() { return clientAddress.sin_port; }
};