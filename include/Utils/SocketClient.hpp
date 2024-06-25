#pragma once

#include <sys/types.h>
#include <netinet/in.h>

#define SOCKET_BUFFER_SIZE 8096

class SocketClient
{
private:
    int client_fd;
    struct sockaddr_in server_address;

public:
    SocketClient(const char *hostname, int port);
    SocketClient(){};
    ~SocketClient(){};
    void connect();
    void write(const char buffer[SOCKET_BUFFER_SIZE]);
    void read(char buffer[SOCKET_BUFFER_SIZE]);
    void close();

};