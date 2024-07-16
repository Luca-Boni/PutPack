#pragma once

#include "Utils/Mutex.hpp"
#include <sys/types.h>
#include <netinet/in.h>

#define SOCKET_BUFFER_SIZE 4096
#define MAX_CONNECT_TRIES 100

namespace SocketClientError{
    const unsigned char NONE = 0b00000000;
    const unsigned char CANT_CONNECT = 0b00000001;
    const unsigned char CANT_READ = 0b00000010;
    const unsigned char CANT_WRITE = 0b0000010;
};

typedef unsigned char SocketClientErrors;

class SocketClient
{
private:
    int client_fd;
    struct sockaddr_in server_address;
    bool connected;
    SocketClientErrors errors;
    Mutex readMutex;
    Mutex writeMutex;
    Mutex connectMutex;

public:
    SocketClient(const char *hostname, int port);
    SocketClient(){};
    ~SocketClient(){};
    bool isConnected();
    SocketClientErrors getErrors();
    void connect();
    void write(const char* buffer);
    char* read();
    void close();
};