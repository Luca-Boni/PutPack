#include "Utils/FileMonitor.hpp"
#include <iostream>
#include <sys/stat.h>

#define FILE_CHANGE IN_CLOSE_WRITE | IN_MOVED_TO //| IN_CREATE
#define FILE_REMOVE IN_DELETE | IN_MOVED_FROM

bool FileMonitor::file_exists(const std::string filename)
{
    struct stat buffer;
    return (stat((monitoredFolder + "/" + filename).c_str(), &buffer) == 0);
}

FileMonitor::FileMonitor() : Thread(std::bind(&FileMonitor::execute, this, std::placeholders::_1), NULL)
{
    shouldStop = false;
}

FileMonitor::FileMonitor(std::string monitoredFolder) : Thread(std::bind(&FileMonitor::execute, this, std::placeholders::_1), NULL)
{
    this->monitoredFolder = monitoredFolder;
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

    int wd = inotify_add_watch(fd, monitoredFolder.c_str(), FILE_CHANGE | FILE_REMOVE);
    if (wd == -1)
        std::cerr << "Error adding watch to file" << std::endl;
    else
        watchedFolder = wd;

    while (!shouldStop)
    {
        int length;
        length = read(fd, buffer, EVENT_BUF_LEN);

        if (length < 0)
            std::cerr << "Error while reading file monitor event" << std::endl;

        int i = 0;
        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                if (event->mask & (FILE_CHANGE | FILE_REMOVE))
                {
                    if (file_exists(event->name))
                    {
                        if (event->mask & IN_ISDIR)
                            std::cout << "Directory " << event->name << " modified." << std::endl;
                        else
                            std::cout << "File " << event->name << " modified." << std::endl;
                    }
                    else
                    {
                        if (event->mask & IN_ISDIR)
                            std::cout << "Directory " << event->name << " deleted." << std::endl;
                        else
                            std::cout << "File " << event->name << " deleted." << std::endl;
                    }
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

    inotify_rm_watch(fd, watchedFolder);

    close(fd);
}

void FileMonitor::stop()
{
    inotify_rm_watch(fd, watchedFolder);
    close(fd);
    shouldStop = true;
    Thread::stop();
}

// void FileMonitor::addWatch(const char *path, uint32_t mask)
// {
//     int wd = inotify_add_watch(fd, path, mask);
//     if (wd == -1)
//         std::cerr << "Error adding watch to file" << std::endl;
//     else
//         watchedItems.push_back(wd);
// }