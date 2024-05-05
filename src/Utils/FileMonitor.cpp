#include "Utils/FileMonitor.hpp"
#include <iostream>

#define FILE_CHANGE IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE
#define FILE_REMOVE IN_DELETE | IN_MOVED_FROM


FileMonitor::FileMonitor() : Thread(std::bind(&FileMonitor::execute, this, std::placeholders::_1), NULL)
{
    shouldStop = false;
}

FileMonitor::FileMonitor(std::vector<std::string> monitoredItems) : Thread(std::bind(&FileMonitor::execute, this, std::placeholders::_1), NULL)
{
    this->monitoredItems = monitoredItems;
    shouldStop = false;
}

FileMonitor::~FileMonitor()
{
    shouldStop = true;
}

void FileMonitor::execute(void *args)
{
    fd = inotify_init();
    if (fd < 0)
        std::cerr << "Error initializing inotify" << std::endl;

    for (std::string monitoredItem : monitoredItems)
    {
        int wd = inotify_add_watch(fd, monitoredItem.c_str(), FILE_CHANGE | FILE_REMOVE);
        if (wd == -1)
            std::cerr << "Error adding watch to file" << std::endl;
        else
            watchedItems.push_back(wd);
    }

    while (!shouldStop)
    {
        int length;
        length = read(fd, buffer, EVENT_BUF_LEN);

        if (length < 0)
        {
            std::cerr << "Error while reading file monitor event" << std::endl;
        }

        int i = 0;
        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                // Prioritizes deletion over modification
                // If both happen in the same event, it means the file was deleted
                if (event->mask & FILE_REMOVE)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        std::cout << "Directory " << event->name << " deleted." << std::endl;
                    }
                    else
                    {
                        std::cout << "File " << event->name << " deleted." << std::endl;
                    }
                }
                else if (event->mask & FILE_CHANGE)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        std::cout << "Directory " << event->name << " modified." << std::endl;
                    }
                    else
                    {
                        std::cout << "File " << event->name << " modified." << std::endl;
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    for (int watchedItem : watchedItems)
    {
        inotify_rm_watch(fd, watchedItem);
    }

    close(fd);
}

void FileMonitor::stop()
{
    shouldStop = true;
}

void FileMonitor::addWatch(const char *path, uint32_t mask)
{
    int wd = inotify_add_watch(fd, path, mask);
    if (wd == -1)
        std::cerr << "Error adding watch to file" << std::endl;
    else
        watchedItems.push_back(wd);
}