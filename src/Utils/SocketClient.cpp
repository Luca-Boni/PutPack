#include "Utils/SocketClient.hpp"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

SocketClient::SocketClient(const char *hostname, int port)
{
    struct hostent *server;
    struct sockaddr_in server_address;

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

    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "Error while connecting to server." << std::endl;
    }
}

void SocketClient::write(const char *buffer, int size)
{
    ::write(client_fd, buffer, size);
    if (size < 0)
    {
        std::cerr << "Error while writing to socket." << std::endl;
    }
}

void SocketClient::read(char buffer[BUFFER_SIZE])
{
    int n = ::read(client_fd, buffer, BUFFER_SIZE);
    if (n < 0)
    {
        std::cerr << "Error while reading from socket." << std::endl;
    }
}

void SocketClient::close()
{
    ::close(client_fd);
}