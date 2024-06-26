#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketClient.hpp"
#include "Utils/FileMonitorProtocol.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <string>
#include <vector>

#define EVENT_SIZE sizeof(struct inotify_event)
#define EVENT_BUF_LEN 1024*(EVENT_SIZE + 16)

class FileMonitor : public Thread
{
private:
    int fd;
    int watchedFolder;
    std::string monitoredFolder;
    char buffer[EVENT_BUF_LEN];
    bool shouldStop;
    SocketClient socket;
    void* execute(void* dummmy);
    bool file_exists(const std::string filename);

public:
    FileMonitor();
    FileMonitor(std::string monitoredFolder, int serverPort, const std::string hostAddress);
    ~FileMonitor();
    void stop();
};