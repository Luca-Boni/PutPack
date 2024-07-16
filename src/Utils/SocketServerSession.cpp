#include "Utils/SocketServerSession.hpp"
#include <cstring>
#include <iostream>
#include <unistd.h>

SocketServerSession::SocketServerSession(std::tuple<socket_t, struct sockaddr_in> sessionInfo)
{
    this->reader_fd = std::get<0>(sessionInfo);
    this->clientAddress = std::get<1>(sessionInfo);
    this->writeMutex = Mutex();
    this->readMutex = Mutex();
}

char* SocketServerSession::read()
{
    readMutex.lock();
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int total_read = 0;

    while (total_read < SOCKET_BUFFER_SIZE)
    {
        int read = 0;
        if ((read = ::read(reader_fd, buffer + total_read, SOCKET_BUFFER_SIZE - total_read)) < 0)
        {
            std::cerr << "Error while reading from socket." << std::endl;
            break;
        }
        else
            total_read += read;
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