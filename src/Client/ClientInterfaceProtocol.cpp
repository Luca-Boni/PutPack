#include "Client/ClientInterfaceProtocol.hpp"
#include "Utils/Protocol.hpp"

#include <cstring>
#include <iostream>

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

char *FileChangeCommandMsg::encode()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    buffer[0] = INTERFACE_COMMAND_MSG;
    buffer[1] = static_cast<unsigned char>(command);
    int offset = 2 * sizeof(unsigned char);

    char cutFilename[FILENAME_SIZE] = {0};
    strcpy(buffer + offset, filename.substr(0, FILENAME_SIZE).c_str());

    offset += FILENAME_SIZE;
    strcpy(buffer + offset, path.c_str());

    return buffer;
}

void FileChangeCommandMsg::decode(const char *buffer)
{
    command = static_cast<InterfaceCommand>(buffer[1]);

    int offset = 2 * sizeof(unsigned char);

    char cutFilename[FILENAME_SIZE] = {0};
    strcpy(cutFilename, buffer + offset);
    filename = std::string(cutFilename);
    offset += FILENAME_SIZE;

    char cpath[SOCKET_BUFFER_SIZE - offset] = {0};
    strcpy(cpath, buffer + offset);
    path = std::string(cpath);
}