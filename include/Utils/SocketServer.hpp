#pragma once

#include <sys/types.h>
#include <netinet/in.h>

#define BUFFER_SIZE 8096

typedef int socket_t;

class SocketServer
{
private:
    socket_t server_fd;
    int valread;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];

public:
    SocketServer(int port);
    SocketServer(){};
    ~SocketServer(){};
    socket_t listenAndAccept();
    void close();
};
