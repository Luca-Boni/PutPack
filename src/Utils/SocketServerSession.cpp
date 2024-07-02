#include "Utils/SocketServerSession.hpp"
#include <cstring>
#include <iostream>
#include <unistd.h>

SocketServerSession::SocketServerSession(socket_t reader_fd) : reader_fd(reader_fd)
{
    this->writeMutex = Mutex();
    this->readMutex = Mutex();
}

char* SocketServerSession::read()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    readMutex.lock();
    if (::read(reader_fd, buffer, SOCKET_BUFFER_SIZE) < 0)
    {
        std::cerr << "Error while reading from socket." << std::endl;
    }
    readMutex.unlock();
    return buffer;
}

void SocketServerSession::write(const char* message)
{
    writeMutex.lock();
    if (::write(reader_fd, message, SOCKET_BUFFER_SIZE) < 0)
    {
        std::cerr << "Error while writing to socket." << std::endl;
    }
    writeMutex.unlock();
}

void SocketServerSession::close()
{
    ::close(reader_fd);
}