#pragma once

#include "Utils/Protocol.hpp"
#include "Utils/SocketClient.hpp"
#include <cstring>
#include <sys/inotify.h>

#define IN_FILE_CHANGE IN_CLOSE_WRITE | IN_MOVED_TO //| IN_CREATE
#define IN_FILE_REMOVE IN_DELETE | IN_MOVED_FROM

enum FileMonitorProtocol : unsigned char
{
    FMP_FILE_CHANGE = 1,
    FMP_FILE_REMOVE = 2
};

struct FileMonitorMsg
{
    int event;
    char filename[FILENAME_SIZE];

    FileMonitorMsg(int event, const char filename[])
    {
        this->event = event;
        strcpy(this->filename, filename);
    }
    FileMonitorMsg()
    {
        event = 0;
        strcpy(filename, "");
    }
    char *encode();
    void decode(const char *buffer);
};