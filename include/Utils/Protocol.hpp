#pragma once

#include "Utils/SocketServerSession.hpp"
#include "Utils/SocketClient.hpp"
#include <cstring>
#include <string>
#include <vector>

#define SERVER_DEAD               (unsigned char)0
#define STOP_MSG                  (unsigned char)1
#define FILE_MONITOR_MSG          (unsigned char)2
#define FILE_READ_MSG             (unsigned char)3
#define FILE_WRITE_MSG            (unsigned char)4
#define SYNC_MSG                  (unsigned char)5
#define NEW_CLIENT_MSG            (unsigned char)6
#define END_CLIENT_MSG            (unsigned char)7
#define FILE_DELETE_MSG           (unsigned char)8 // file deleted naturally
#define FILE_UPLOAD_MSG           (unsigned char)9
#define CLIENT_CONNECTED_MSG      (unsigned char)10
#define REJECT_CONNECT_MSG        (unsigned char)11
#define INTERFACE_COMMAND_MSG     (unsigned char)12
#define FILE_MODIFIED_MSG         (unsigned char)13
#define FILE_DOWNLOAD_MSG         (unsigned char)14
#define FILE_DEL_CMD_MSG          (unsigned char)15 // file deleted via command
#define LIST_SERVER_FILES_MSG     (unsigned char)16
#define NEW_SERVER_MSG            (unsigned char)17
#define PRIMARY_ALIVE_MSG         (unsigned char)18
#define CHECK_PRIMARY_ALIVE_MSG   (unsigned char)19
#define OK_MSG                    (unsigned char)20
#define SYNC_FINISHED_MSG         (unsigned char)21
#define NEXT_IN_RING_MSG          (unsigned char)22
#define PREV_IN_RING_MSG          (unsigned char)23
#define ELECTION_MSG              (unsigned char)24
#define ELECTED_MSG               (unsigned char)25
#define SET_TO_PRIMARY_MSG        (unsigned char)26
#define SET_OWN_ADDRESS_MSG       (unsigned char)27

#define SEND_CLI_REDIRECT_MSG     (unsigned char)80
#define RECV_CLI_REDIRECT_MSG     (unsigned char)160

#define FILENAME_SIZE 128 * sizeof(char)
#define USERNAME_SIZE FILENAME_SIZE

std::string filenameFromPath(const std::string &path);

struct ServerInfo
{
    int id;
    std::string address;
    int port;
};

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
    char clientAddress[USERNAME_SIZE];
    int FEPort;

    ClientConnectedMsg(const char username[], SocketServerSession *clientSocket = NULL, int FEPort = 0)
    {
        strcpy(this->username, username);
        this->clientSocket = clientSocket;
        if (clientSocket != NULL)
        {
            strcpy(this->clientAddress, clientSocket->getClientIP().c_str());            
            this->FEPort = FEPort;
        }
        else
        {
            strcpy(this->clientAddress, "");
            this->FEPort = FEPort;
        }
    }
    ClientConnectedMsg()
    {
        strcpy(this->username, "");
        clientSocket = NULL;
        strcpy(this->clientAddress, "");
        FEPort = 0;
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

enum class InterfaceCommand : unsigned char
{
    UPLOAD,
    DOWNLOAD,
    DELETE,
    LIST_SERVER,
    LIST_CLIENT,
    GET_SYNC_DIR,
    EXIT
};

class InterfaceCommandMsg
{
public:
    InterfaceCommand command;

    InterfaceCommandMsg() {}
    InterfaceCommandMsg(InterfaceCommand command) : command(command) {}
    char *encode();
    void decode(const char *buffer);
};

class ListServerCommandMsg : public InterfaceCommandMsg
{
public:
    unsigned long long clientId;
    std::string data;

    ListServerCommandMsg() : InterfaceCommandMsg(InterfaceCommand::LIST_SERVER) {}
    char *encode();
    void decode(const char *buffer);
};

struct NewServerMsg
{
    int id;
    std::string address;
    int port;
    SocketClient *socket;
    SocketServerSession *socketSession;

    NewServerMsg(int id, const std::string &address, int port, SocketClient* socket) : id(id), address(address), port(port), socket(socket), socketSession(NULL) {}
    NewServerMsg() : id(0), address(""), port(0), socket(NULL), socketSession(NULL) {}
    char *encode();
    void decode(const char *buffer);
};

struct NextInRingMsg
{
    int id;
    std::string address;
    int port;
    std::string ownAddress;
    int ownPort;

    NextInRingMsg(int id, const std::string &address, int port, const std::string &ownAddress, int ownPort) : id(id), address(address), port(port), ownAddress(ownAddress), ownPort(ownPort) {}
    NextInRingMsg() : id(0), address(""), port(0), ownAddress(""), ownPort(0) {}
    char *encode();
    void decode(const char *buffer);
};

struct ElectionMsg
{
    int id;

    ElectionMsg(int id) : id(id) {}
    ElectionMsg() : id(0) {}
    char *encode();
    void decode(const char *buffer);
};

struct ElectedMsg
{
    int id;
    std::string address;
    int port;

    ElectedMsg(int id) : id(id) {}
    ElectedMsg() : id(0) {}
    char *encode();
    void decode(const char *buffer);
};

struct SetOwnAddressMsg
{
    std::string address;

    SetOwnAddressMsg(const std::string &address) : address(address) {}
    SetOwnAddressMsg() : address("") {}
    char *encode();
    void decode(const char *buffer);
};