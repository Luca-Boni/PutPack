#pragma once

#include "Utils/SocketServer.hpp"

class SocketServerSession
{
private:
    socket_t reader_fd;
    char buffer[BUFFER_SIZE];

public:
    SocketServerSession(socket_t reader_fd);
    ~SocketServerSession(){};
    void read(char buffer[BUFFER_SIZE]);
    void write(const char *buffer, int size);
    void close();
};