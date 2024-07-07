#include "Utils/Protocol.hpp"
#include "Server/ClientUserProtocol.hpp"
#include <cstring>

char* NewClientMsg::encode()
{
    char* buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = NEW_CLIENT_MSG;
    offset += 1;

    memcpy(buffer + offset, &clientId, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    memcpy(buffer + offset, clientSocket, sizeof(SocketServerSession*));

    return buffer;
}

void NewClientMsg::decode(const char* buffer)
{
    clientId = *((unsigned long long*)(buffer + 1));
    clientSocket = *((SocketServerSession**)(buffer + 1 + sizeof(unsigned long long)));
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