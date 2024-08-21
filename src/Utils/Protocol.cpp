#include "Utils/Protocol.hpp"
#include "Utils/SocketClient.hpp"
#include <cstring>

std::string filenameFromPath(const std::string &path)
{
    size_t found = path.find_last_of("/\\");
    return path.substr(found + 1);
}

char *SyncAllMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = SYNC_MSG;
    offset += 1;

    memcpy(buffer + offset, &clientId, sizeof(unsigned long long));

    return buffer;
}

void SyncAllMsg::decode(const char *buffer)
{
    clientId = *((unsigned long long *)(buffer + 1));
}

char *ClientConnectedMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = CLIENT_CONNECTED_MSG;
    offset += 1;

    memcpy(buffer + offset, username, USERNAME_SIZE);
    offset += USERNAME_SIZE;

    memcpy(buffer + offset, &clientSocket, sizeof(SocketServerSession *));
    offset += sizeof(SocketServerSession *);

    memcpy(buffer + offset, clientAddress, USERNAME_SIZE);
    offset += USERNAME_SIZE;

    memcpy(buffer + offset, &FEPort, sizeof(int));

    return buffer;
}

void ClientConnectedMsg::decode(const char *buffer)
{
    int offset = 1;
    memcpy(username, buffer + offset, USERNAME_SIZE);
    offset += USERNAME_SIZE;

    clientSocket = *(SocketServerSession **)(buffer + offset);
    offset += sizeof(SocketServerSession *);

    memcpy(clientAddress, buffer + offset, USERNAME_SIZE);
    offset += USERNAME_SIZE;

    FEPort = *((int *)(buffer + offset));
}

void RejectConnectMsg::decode(const char *buffer)
{
    messageSize = *((int *)(buffer + 1));
    message = new char[messageSize];
    strcpy(message, buffer + 1 + sizeof(int));
}

char *RejectConnectMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = REJECT_CONNECT_MSG;
    offset += 1;

    memcpy(buffer + offset, &messageSize, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, message, messageSize);

    return buffer;
}

char *EndClientMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = END_CLIENT_MSG;
    offset += 1;

    memcpy(buffer + offset, &clientId, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    strcpy(buffer + offset, username);

    return buffer;
}

void EndClientMsg::decode(const char *buffer)
{
    clientId = *((unsigned long long *)(buffer + 1));
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
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = INTERFACE_COMMAND_MSG;
    offset += sizeof(unsigned char);

    buffer[offset] = static_cast<unsigned char>(command);
    offset += sizeof(unsigned char);

    memcpy(buffer + offset, &clientId, sizeof(unsigned long long));
    offset += sizeof(unsigned long long);

    char cutData[SOCKET_BUFFER_SIZE - 2];
    strcpy(cutData, data.substr(0, SOCKET_BUFFER_SIZE - 2).c_str());
    strcpy(buffer + offset, cutData);

    return buffer;
}

void ListServerCommandMsg::decode(const char *buffer)
{
    char cutData[SOCKET_BUFFER_SIZE] = {0};

    int offset = 2;
    clientId = *(unsigned long long *)(buffer + offset);
    offset += sizeof(unsigned long long);

    memcpy(cutData, (buffer + offset), SOCKET_BUFFER_SIZE - 2);
    data = std::string(cutData);
}

char *NewServerMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = NEW_SERVER_MSG;
    offset += 1;

    memcpy(buffer + offset, &id, sizeof(int));
    offset += sizeof(int);

    strcpy(buffer + offset, address.substr(0, 50).c_str());
    offset += 50;

    memcpy(buffer + offset, &port, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &socket, sizeof(SocketClient *));
    offset += sizeof(SocketClient *);

    memcpy(buffer + offset, &socketSession, sizeof(SocketServerSession *));

    return buffer;
}

void NewServerMsg::decode(const char *buffer)
{
    int offset = 1;

    id = *(int *)(buffer + offset);
    offset += sizeof(int);

    address = std::string(buffer + offset);
    offset += 50;

    port = *(int *)(buffer + offset);
    offset += sizeof(int);

    socket = *(SocketClient **)(buffer + offset);
    offset += sizeof(SocketClient *);

    socketSession = *(SocketServerSession **)(buffer + offset);
}

char *NextInRingMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = NEXT_IN_RING_MSG;
    offset += 1;

    memcpy(buffer + offset, &id, sizeof(int));
    offset += sizeof(int);

    strcpy(buffer + offset, address.substr(0, 50).c_str());
    offset += 50;

    memcpy(buffer + offset, &port, sizeof(int));
    offset += sizeof(int);

    strcpy(buffer + offset, ownAddress.substr(0, 50).c_str());
    offset += 50;

    memcpy(buffer + offset, &ownPort, sizeof(int));
    offset += sizeof(int);

    return buffer;
}

void NextInRingMsg::decode(const char *buffer)
{
    int offset = 1;

    id = *(int *)(buffer + offset);
    offset += sizeof(int);

    address = std::string(buffer + offset);
    offset += 50;

    port = *(int *)(buffer + offset);
    offset += sizeof(int);

    ownAddress = std::string(buffer + offset);
    offset += 50;

    ownPort = *(int *)(buffer + offset);
    offset += sizeof(int);
}

char *ElectionMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = ELECTION_MSG;
    offset += 1;

    memcpy(buffer + offset, &id, sizeof(int));
    offset += sizeof(int);

    return buffer;
}

void ElectionMsg::decode(const char *buffer)
{
    int offset = 1;

    id = *(int *)(buffer + offset);
    offset += sizeof(int);
}

char *ElectedMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = ELECTED_MSG;
    offset += 1;

    memcpy(buffer + offset, &id, sizeof(int));
    offset += sizeof(int);

    strcpy(buffer + offset, address.substr(0, 50).c_str());
    offset += 50;

    memcpy(buffer + offset, &port, sizeof(int));
    offset += sizeof(int);

    return buffer;
}

void ElectedMsg::decode(const char *buffer)
{
    int offset = 1;

    id = *(int *)(buffer + offset);
    offset += sizeof(int);

    address = std::string(buffer + offset);
    offset += 50;

    port = *(int *)(buffer + offset);
    offset += sizeof(int);
}

char *SetOwnAddressMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    int offset = 0;

    buffer[offset] = SET_OWN_ADDRESS_MSG;
    offset += 1;

    strcpy(buffer + offset, address.substr(0, 50).c_str());
    offset += 50;

    return buffer;
}

void SetOwnAddressMsg::decode(const char *buffer)
{
    int offset = 1;

    address = std::string(buffer + offset);
    offset += 50;
}