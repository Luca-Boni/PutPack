#pragma once

#include "Utils/Thread.hpp"
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
    void execute(void* dummmy);
    bool file_exists(const std::string filename);

public:
    FileMonitor();
    FileMonitor(std::string monitoredFolder);
    ~FileMonitor();
    void stop();

    // void addWatch(const char *path, uint32_t mask);
};