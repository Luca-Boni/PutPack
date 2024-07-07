#pragma once

#include "Utils/SocketClient.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/FileHandlerProtocol.hpp"

struct NewClientMsg
{
    unsigned long long clientId;
    SocketServerSession* clientSocket;

    NewClientMsg(unsigned long long clientId, SocketServerSession* clientSocket) : clientId(clientId), clientSocket(clientSocket) {}
    NewClientMsg() : clientId(0), clientSocket(NULL) {}
    char* encode();
    void decode(const char* buffer);
};

struct EndClientMsg
{
    unsigned long long clientId;
    char username[USERNAME_SIZE];

    EndClientMsg(unsigned long long clientId, char username[USERNAME_SIZE]) : clientId(clientId) {strcpy(this->username, username);}
    EndClientMsg() : clientId(0), username(""){}
    char* encode();
    void decode(const char* buffer);
};