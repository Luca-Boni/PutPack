#include "Utils/SocketServer.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

SocketServer::SocketServer(int port)
{
    address_len = sizeof(address);

    socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket == 0)
    {
        std::cerr << "Error while creating socket." << std::endl;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(address.sin_zero), 8);

    if (bind(socket, (struct sockaddr *)&address, address_len) < 0)
    {
        std::cerr << "Error while binding socket." << std::endl;
    }

    if (getsockname(socket, (struct sockaddr *)&address, &address_len) < 0)
    {
        std::cerr << "Error while getting socket info." << std::endl;
    }

    this->port = ntohs(address.sin_port);
}

std::tuple<socket_t, struct sockaddr_in> SocketServer::listenAndAccept()
{
    if (::listen(socket, 20) < 0)
    {
        std::cerr << "Error while listening." << std::endl;
    }

    int clilen = sizeof(struct sockaddr_in);
    struct sockaddr_in client_address;
    socket_t new_socket;
    if ((new_socket = accept(socket, (struct sockaddr *)&client_address, (socklen_t *)&clilen)) < 0)
    {
        std::cerr << "Error while accepting connection." << std::endl;
    }

    return std::make_tuple(new_socket, client_address);
}

void SocketServer::close()
{
    ::close(socket);
}