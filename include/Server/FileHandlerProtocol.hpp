#pragma once

#include "Utils/Protocol.hpp"
#include "Utils/SocketClient.hpp"
#include <cstring>
#include <sys/inotify.h>

#define FILENAME_SIZE 128*sizeof(char)
#define FILE_BUFFER_SIZE SOCKET_BUFFER_SIZE - sizeof(char) - sizeof(int) - FILENAME_SIZE

enum class FileHandlerMode : unsigned char
{
    READ,
    WRITE
};

namespace FileHandlerProtocol
{
    char *encode(const char* filename, const char* buffer, const int size);
}