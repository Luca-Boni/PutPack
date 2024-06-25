#include "Utils/SocketServerSession.hpp"
#include <cstring>
#include <iostream>
#include <unistd.h>

SocketServerSession::SocketServerSession(socket_t reader_fd)
{
    this->reader_fd = reader_fd;
}

void SocketServerSession::read(char buffer[SOCKET_BUFFER_SIZE])
{
    bzero(buffer, SOCKET_BUFFER_SIZE);
    if (::read(reader_fd, buffer, SOCKET_BUFFER_SIZE) < 0)
    {
        std::cerr << "Error while reading from socket." << std::endl;
    }
}

void SocketServerSession::write(const char message[SOCKET_BUFFER_SIZE])
{
    if (::write(reader_fd, message, SOCKET_BUFFER_SIZE) < 0)
    {
        std::cerr << "Error while writing to socket." << std::endl;
    }
}

void SocketServerSession::close()
{
    ::close(reader_fd);
}