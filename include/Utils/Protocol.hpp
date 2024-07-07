#pragma once

#include "Utils/SocketServerSession.hpp"
#include <cstring>

#define STOP_MSG             (unsigned char)1
#define FILE_MONITOR_MSG     (unsigned char)2
#define FILE_READ_MSG        (unsigned char)3
#define FILE_WRITE_MSG       (unsigned char)4
#define SYNC_MSG             (unsigned char)5
#define NEW_CLIENT_MSG       (unsigned char)6
#define END_CLIENT_MSG       (unsigned char)7
#define FILE_DELETE_MSG      (unsigned char)8
#define FILE_UPLOAD_MSG      (unsigned char)9
#define CLIENT_CONNECTED_MSG (unsigned char)10
#define REJECT_CONNECT_MSG   (unsigned char)11

#define FILENAME_SIZE 128 * sizeof(char)
#define USERNAME_SIZE FILENAME_SIZE

struct SyncAllMsg
{
    unsigned long long clientId;

    SyncAllMsg(unsigned long long clientId) : clientId(clientId) {}
    SyncAllMsg() : clientId(0) {}
    char *encode();
    void decode(const char *buffer);
};

struct ClientConnectedMsg
{
    char username[USERNAME_SIZE];
    SocketServerSession *clientSocket;

    ClientConnectedMsg(char username[], SocketServerSession *clientSocket)
    {
        strcpy(this->username, username);
        this->clientSocket = clientSocket;
    }
    ClientConnectedMsg()
    {
        strcpy(this->username, "");
        clientSocket = NULL;
    }
    char *encode();
    void decode(const char *buffer);
};

struct RejectConnectMsg
{
    int messageSize;
    char *message;

    RejectConnectMsg(const char *message)
    {
        messageSize = strlen(message);
        strcpy(this->message, message);
    }
    RejectConnectMsg()
    {
        messageSize = 0;
        strcpy(this->message, "");
    }
    char *encode();
    void decode(const char *buffer);
};