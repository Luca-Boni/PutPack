#include "Utils/FileMonitorProtocol.hpp"

char* FileMonitorProtocol::encode(int event, const char *filename)
{
    char *buffer = new char[SOCKET_BUFFER_SIZE];
    buffer[0] = FILE_MONITOR_MSG;
    buffer[1] = event;
    strcpy(&buffer[2], filename);

    return buffer;
}