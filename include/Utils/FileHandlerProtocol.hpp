#pragma once

#include "Utils/Protocol.hpp"
#include "Utils/SocketClient.hpp"
#include <cstring>
#include <sys/inotify.h>

#define FILE_BUFFER_SIZE SOCKET_BUFFER_SIZE - (sizeof(char) + sizeof(unsigned long long) + FILENAME_SIZE + sizeof(unsigned int))

enum class FileHandlerMode : unsigned char
{
    READ,
    WRITE,
    DELETE
};

struct FileHandlerMessage
{
    unsigned long long clientId;
    char* filename;
    unsigned int size;
    char* data;

    FileHandlerMessage();
    FileHandlerMessage(unsigned long long clientId, const char *filename, unsigned int size, const char *data);
    ~FileHandlerMessage();
    char *encode();
    void decode(const char *buffer);
};