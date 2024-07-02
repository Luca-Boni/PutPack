#pragma once

#include "Utils/Mutex.hpp"
#include "Utils/SocketServer.hpp"

class SocketServerSession
{
private:
    socket_t reader_fd;
    Mutex writeMutex;
    Mutex readMutex;

public:
    SocketServerSession(){};
    SocketServerSession(socket_t reader_fd);
    ~SocketServerSession(){};
    void write(const char* buffer);
    char* read();
    void close();
};