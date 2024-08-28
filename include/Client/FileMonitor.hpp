#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketClient.hpp"
#include "Client/FileMonitorProtocol.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <string>
#include <vector>
#include <unordered_set>

#define EVENT_SIZE sizeof(struct inotify_event)
#define EVENT_BUF_LEN 1024 * (EVENT_SIZE + 16)

class FileMonitor : public Thread
{
private:
    int fd;
    int watchedFolder;
    std::string monitoredFolder;
    char buffer[EVENT_BUF_LEN];
    bool shouldStop;
    SocketClient *socket;
    std::unordered_set<std::string> disabledFiles;

    void *execute(void *dummmy);

    bool file_exists(const std::string filename);

public:
    FileMonitor();
    FileMonitor(const std::string monitoredFolder, SocketClient *socket);
    ~FileMonitor();
    void stop();

    void disableFile(const std::string filename);
    void enableFile(const std::string filename);
};