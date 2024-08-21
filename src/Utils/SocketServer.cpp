#include "Utils/SocketServer.hpp"
#include "Utils/Logger.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

SocketServer::SocketServer(int port)
{
    address_len = sizeof(address);

    socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket == 0)
    {
        Logger::log("Error while creating socket.");
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(address.sin_zero), 8);

    if (bind(socket, (struct sockaddr *)&address, address_len) < 0)
    {
        Logger::log("Error while binding socket to port " + std::to_string(port));
    }

    if (getsockname(socket, (struct sockaddr *)&address, &address_len) < 0)
    {
        Logger::log("Error while getting socket info for port " + std::to_string(port));
    }

    this->port = ntohs(address.sin_port);
}

std::tuple<socket_t, struct sockaddr_in> SocketServer::listenAndAccept()
{
    if (::listen(socket, 20) < 0)
    {
        Logger::log("Error while listening on port " + std::to_string(port));
    }

    int clilen = sizeof(struct sockaddr_in);
    struct sockaddr_in client_address;
    socket_t new_socket;
    if ((new_socket = accept(socket, (struct sockaddr *)&client_address, (socklen_t *)&clilen)) < 0)
    {
        Logger::log("Error while accepting connection on port " + std::to_string(port) + "; Client info: " + std::string(inet_ntoa(client_address.sin_addr)) + ":" + std::to_string(client_address.sin_port));
    }

    return std::make_tuple(new_socket, client_address);
}

void SocketServer::close()
{
    Logger::log("Closing socket on port " + std::to_string(port));
    ::close(socket);
}