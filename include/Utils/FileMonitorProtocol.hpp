#pragma once

#include "Utils/ClientProtocol.hpp"
#include "Utils/SocketClient.hpp"
#include <cstring>
#include <sys/inotify.h>

#define IN_FILE_CHANGE IN_CLOSE_WRITE | IN_MOVED_TO //| IN_CREATE
#define IN_FILE_REMOVE IN_DELETE | IN_MOVED_FROM

namespace FileMonitorProtocol
{
    enum FileMonitorProtocol : unsigned char
    {
        FMP_FILE_CHANGE = 1,
        FMP_FILE_REMOVE = 2
    };

    char* encode(int mask, const char *filename);
}