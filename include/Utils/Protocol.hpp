#pragma once

#include "Utils/SocketServerSession.hpp"
#include "Client/ClientInterfaceProtocol.hpp"
#include <cstring>
#include <string>
#include <vector>

#define SERVER_DEAD           (unsigned char)0
#define STOP_MSG              (unsigned char)1
#define FILE_MONITOR_MSG      (unsigned char)2
#define FILE_READ_MSG         (unsigned char)3
#define FILE_WRITE_MSG        (unsigned char)4
#define SYNC_MSG              (unsigned char)5
#define NEW_CLIENT_MSG        (unsigned char)6
#define END_CLIENT_MSG        (unsigned char)7
#define FILE_DELETE_MSG       (unsigned char)8 // file deleted naturally
#define FILE_UPLOAD_MSG       (unsigned char)9
#define CLIENT_CONNECTED_MSG  (unsigned char)10
#define REJECT_CONNECT_MSG    (unsigned char)11
#define INTERFACE_COMMAND_MSG (unsigned char)12
#define FILE_MODIFIED_MSG     (unsigned char)13
#define FILE_DOWNLOAD_MSG     (unsigned char)14
#define FILE_DEL_CMD_MSG      (unsigned char)15 // file deleted via command
#define LIST_SERVER_FILES_MSG (unsigned char)16

#define FILENAME_SIZE 128 * sizeof(char)
#define USERNAME_SIZE FILENAME_SIZE

std::string filenameFromPath(const std::string &path);

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

    ClientConnectedMsg(const char username[], SocketServerSession *clientSocket = NULL)
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
        this->message = new char[messageSize];
        strcpy(this->message, message);
    }
    RejectConnectMsg()
    {
        messageSize = 0;
        this->message = NULL;
    }
    char *encode();
    void decode(const char *buffer);
};

struct EndClientMsg
{
    unsigned long long clientId;
    char username[USERNAME_SIZE];

    EndClientMsg(unsigned long long clientId, const char username[USERNAME_SIZE]) : clientId(clientId) {strcpy(this->username, username);}
    EndClientMsg() : clientId(0), username(""){}
    char* encode();
    void decode(const char* buffer);
};

class ListServerCommandMsg : public InterfaceCommandMsg
{
public:
    std::vector<std::string> a;

    ListServerCommandMsg() : InterfaceCommandMsg(InterfaceCommand::LIST_SERVER) {}
    char *encode();
    void decode(const char *buffer);
};