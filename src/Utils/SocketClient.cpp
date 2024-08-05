#include "Utils/SocketClient.hpp"
#include "Utils/Logger.hpp"

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
        Logger::log("Error while creating socket to connect to server " + std::string(hostname) + ":" + std::to_string(port));
    }

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        Logger::log("Error while getting host " + std::string(hostname));
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
            Logger::log("Error while connecting to server " + getServerIP() + ":" + std::to_string(getServerPort()) + ". Trying again.");
            tries++;
            nanosleep((const struct timespec[]){{0, (1000 * 1000 * 100)}}, NULL);
        }
        if (tries == MAX_CONNECT_TRIES)
        {
            this->errors |= SocketClientError::CANT_CONNECT;
            Logger::log("Max number of tries reached trying to connect to server " + getServerIP() + ":" + std::to_string(getServerPort()));
        }
        else
        {
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
        Logger::log("Error while writing to socket on server " + getServerIP() + ":" + std::to_string(getServerPort()));
    }
    writeMutex.unlock();
}

char* SocketClient::read()
{
    readMutex.lock();
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int total_read = 0;

    while (total_read < SOCKET_BUFFER_SIZE)
    {
        int read = 0;
        if ((read = ::read(client_fd, buffer + total_read, SOCKET_BUFFER_SIZE - total_read)) < 0)
        {
            Logger::log("Error while reading from socket of server " + getServerIP() + ":" + std::to_string(getServerPort()));
            break;
        }
        else
            total_read += read;
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