#pragma once

#include "Utils/SocketClient.hpp"

struct NewClientMsg
{
    unsigned long long clientId;
    SocketClient* socketClient;

    NewClientMsg(unsigned long long clientId, SocketClient* socketClient) : clientId(clientId), socketClient(socketClient) {}
    NewClientMsg() : clientId(0), socketClient(NULL) {}
    char* encode();
    void decode(const char* buffer);
};

struct EndClientMsg
{
    unsigned long long clientId;

    EndClientMsg(unsigned long long clientId) : clientId(clientId) {}
    EndClientMsg() : clientId(0) {}
    char* encode();
    void decode(const char* buffer);
};