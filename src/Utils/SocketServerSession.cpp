#include "Utils/SocketServerSession.hpp"
#include <cstring>
#include <iostream>
#include <unistd.h>

SocketServerSession::SocketServerSession(socket_t reader_fd)
{
    this->reader_fd = reader_fd;
}

void SocketServerSession::read(char buffer[BUFFER_SIZE])
{
    bzero(buffer, BUFFER_SIZE);
    if (::read(reader_fd, buffer, BUFFER_SIZE))
    {
        std::cerr << "Error while reading from socket." << std::endl;
    }
}

void SocketServerSession::write(const char *message, int size)
{
    if (::write(reader_fd, message, size) < 0)
    {
        std::cerr << "Error while writing to socket." << std::endl;
    }
}

void SocketServerSession::close()
{
    ::close(reader_fd);
}