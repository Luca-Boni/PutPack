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