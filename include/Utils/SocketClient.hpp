#pragma once

#include <sys/types.h>
#include <netinet/in.h>

#define BUFFER_SIZE 8096

class SocketClient
{
private:
    int client_fd;
    char buffer[BUFFER_SIZE];

public:
    SocketClient(const char *hostname, int port);
    SocketClient(){};
    ~SocketClient(){};
    void write(const char *buffer, int size);
    void read(char buffer[BUFFER_SIZE]);
    void close();

};