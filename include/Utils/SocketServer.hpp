#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET_BUFFER_SIZE 8096

typedef int socket_t;

class SocketServer
{
private:
    socket_t server_fd;
    int port;
    int valread;
    struct sockaddr_in address;
    socklen_t address_len;
    char buffer[SOCKET_BUFFER_SIZE];

public:
    SocketServer(int port);
    SocketServer(){};
    ~SocketServer(){};
    socket_t listenAndAccept();
    void close();
    int getPort() { return port; }
};
