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
    std::vector<int> watchedItems;
    std::vector<std::string> monitoredItems;
    char buffer[EVENT_BUF_LEN];
    bool shouldStop;
    void execute(void* dummmy);

public:
    FileMonitor();
    FileMonitor(std::vector<std::string> monitoredItems);
    ~FileMonitor();
    void stop();

    void addWatch(const char *path, uint32_t mask);
};