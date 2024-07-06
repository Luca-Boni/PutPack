#include "Utils/Protocol.hpp"
#include "Utils/SocketClient.hpp"
#include <cstring>

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