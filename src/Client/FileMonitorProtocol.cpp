#include "Client/FileMonitorProtocol.hpp"

char* FileMonitorMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    buffer[0] = FILE_MONITOR_MSG;
    buffer[1] = event;
    strcpy(&buffer[2], filename);

    return buffer;
}

void FileMonitorMsg::decode(const char *buffer)
{
    event = buffer[1];
    strcpy(filename, &buffer[2]);
}