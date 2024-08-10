#include "Utils/Protocol.hpp"
#include "Utils/SocketClient.hpp"
#include <cstring>

std::string filenameFromPath(const std::string &path)
{
    size_t found = path.find_last_of("/\\");
    return path.substr(found + 1);
}

char* SyncAllMsg::encode()
{
    char* buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = SYNC_MSG;
    offset += 1;

    memcpy(buffer + offset, &clientId, sizeof(unsigned long long));

    return buffer;
}

void SyncAllMsg::decode(const char* buffer)
{
    clientId = *((unsigned long long*)(buffer + 1));
}


char* ClientConnectedMsg::encode()
{
    char* buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = CLIENT_CONNECTED_MSG;
    offset += 1;

    memcpy(buffer + offset, username, USERNAME_SIZE);
    offset += USERNAME_SIZE;

    memcpy(buffer + offset, &clientSocket, sizeof(SocketServerSession*));

    return buffer;
}

void ClientConnectedMsg::decode(const char* buffer)
{
    memcpy(username, buffer + 1, USERNAME_SIZE);
    clientSocket = *((SocketServerSession**)(buffer + 1 + USERNAME_SIZE));
}

void RejectConnectMsg::decode(const char* buffer)
{
    messageSize = *((int*)(buffer + 1));
    message = new char[messageSize];
    strcpy(message, buffer + 1 + sizeof(int));
}

char* RejectConnectMsg::encode()
{
    char* buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = REJECT_CONNECT_MSG;
    offset += 1;

    memcpy(buffer + offset, &messageSize, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, message, messageSize);

    return buffer;
}


char* EndClientMsg::encode()
{
    char* buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = END_CLIENT_MSG;
    offset += 1;

    memcpy(buffer + offset, &clientId, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    strcpy(buffer + offset, username);

    return buffer;
}

void EndClientMsg::decode(const char* buffer)
{
    clientId = *((unsigned long long*)(buffer + 1));
    strcpy(username, buffer + 1 + sizeof(unsigned long long));
}

char *InterfaceCommandMsg::encode()
{
    int offset = 0;
    char *buffer = new char[SOCKET_BUFFER_SIZE]();

    buffer[offset] = INTERFACE_COMMAND_MSG;
    offset += sizeof(unsigned char);

    buffer[offset] = static_cast<unsigned char>(command);
    offset += sizeof(unsigned char);

    return buffer;
}

void InterfaceCommandMsg::decode(const char *buffer)
{
    command = static_cast<InterfaceCommand>(buffer[1]);
}

char *ListServerCommandMsg::encode()
{
    char* buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = LIST_SERVER_FILES_MSG;
    offset += 1;

    memcpy(buffer + offset, &clientId, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    char cutData[SOCKET_BUFFER_SIZE - 2];
    strcpy(cutData, data.substr(0, SOCKET_BUFFER_SIZE - 2).c_str());
    strcpy(buffer + offset, cutData);

    return buffer;
}

void ListServerCommandMsg::decode(const char* buffer)
{
    char cutData[SOCKET_BUFFER_SIZE] = {0};

    clientId = *(unsigned long long*)(buffer + 1);

    memcpy(cutData, (buffer + 1 + sizeof(unsigned long long)), SOCKET_BUFFER_SIZE - 2);
    data = std::string(cutData);
}