#include "Utils/SocketClient.hpp"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

SocketClient::SocketClient(const char *hostname, int port)
{
    this->writeMutex = Mutex();
    this->readMutex = Mutex();
    this->connectMutex = Mutex();
    this->connected = false;
    this->errors = SocketClientError::NONE;

    struct hostent *server;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == 0)
    {
        std::cerr << "Error while creating socket." << std::endl;
    }

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        std::cerr << "Error while getting host." << std::endl;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(server_address.sin_zero), 8);
}

void SocketClient::connect()
{
    connectMutex.lock();
    if (!connected)
    {
        int tries = 0;
        while (tries < MAX_CONNECT_TRIES && ::connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            std::cerr << "Error while connecting to server. Trying again." << std::endl;
            tries++;
            nanosleep((const struct timespec[]){{0, (1000 * 1000 * 100)}}, NULL);
        }
        if (tries == MAX_CONNECT_TRIES)
        {
            this->errors |= SocketClientError::CANT_CONNECT;
            std::cerr << "Max number of tries reached." << std::endl;
        }
        else
        {
            std::cout << "Connected to server." << std::endl;
            connected = true;
        }
    }
    connectMutex.unlock();
}

void SocketClient::write(const char* buffer)
{
    writeMutex.lock();
    int size = ::write(client_fd, buffer, SOCKET_BUFFER_SIZE);
    if (size < 0)
    {
        this->errors |= SocketClientError::CANT_WRITE;
        std::cerr << "Error while writing to socket." << std::endl;
    }
    writeMutex.unlock();
}

char* SocketClient::read()
{
    char* buffer = new char[SOCKET_BUFFER_SIZE]();
    readMutex.lock();
    int n = ::read(client_fd, buffer, SOCKET_BUFFER_SIZE);
    if (n < 0)
    {
        this->errors |= SocketClientError::CANT_READ;
        std::cerr << "Error while reading from socket." << std::endl;
    }
    readMutex.unlock();
    return buffer;
}

void SocketClient::close()
{
    connectMutex.lock();
    if (connected)
    {
        ::close(client_fd);
        connected = false;
    }
    connectMutex.unlock();
}

bool SocketClient::isConnected()
{
    return connected;
}

SocketClientErrors SocketClient::getErrors()
{
    return errors;
}