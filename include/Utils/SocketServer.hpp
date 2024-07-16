#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <tuple>

#define SOCKET_BUFFER_SIZE 4096

typedef int socket_t;

class SocketServer
{
private:
    socket_t socket;
    int port;
    int valread;
    struct sockaddr_in address;
    socklen_t address_len;

public:
    SocketServer(int port = 0);
    ~SocketServer(){};
    std::tuple<socket_t, struct sockaddr_in> listenAndAccept();
    void close();
    int getPort() { return port; }
};
