#pragma once

#include "Utils/Mutex.hpp"
#include "Utils/SocketServer.hpp"

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
};