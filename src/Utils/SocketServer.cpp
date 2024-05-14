#include "Utils/SocketServer.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

SocketServer::SocketServer(int port)
{
    address_len = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
    {
        std::cerr << "Error while creating socket." << std::endl;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(address.sin_zero), 8);

    if (bind(server_fd, (struct sockaddr *)&address, address_len) < 0)
    {
        std::cerr << "Error while binding socket." << std::endl;
    }

    if (getsockname(server_fd, (struct sockaddr *)&address, &address_len) < 0)
    {
        std::cerr << "Error while getting socket info." << std::endl;
    }

    this->port = ntohs(address.sin_port);
}

socket_t SocketServer::listenAndAccept()
{
    if (::listen(server_fd, 20) < 0)
    {
        std::cerr << "Error while listening." << std::endl;
    }

    int clilen = sizeof(struct sockaddr_in);
    struct sockaddr_in client_address;
    socket_t new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&clilen)) < 0)
    {
        std::cerr << "Error while accepting connection." << std::endl;
    }

    return new_socket;
}

void SocketServer::close()
{
    ::close(server_fd);
}