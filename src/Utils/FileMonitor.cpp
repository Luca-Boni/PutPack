#include "Utils/FileMonitor.hpp"
#include <iostream>
#include <sys/stat.h>

bool FileMonitor::file_exists(const std::string filename)
{
    struct stat buffer;
    return (stat((monitoredFolder + "/" + filename).c_str(), &buffer) == 0);
}

FileMonitor::FileMonitor(){}

FileMonitor::FileMonitor(const std::string monitoredFolder, const int serverPort, const std::string hostAddress="localhost") : Thread(std::bind(&FileMonitor::execute, this, std::placeholders::_1), NULL)
{
    socket = SocketClient(hostAddress.c_str(), serverPort);
    this->monitoredFolder = monitoredFolder;
    shouldStop = false;
}

FileMonitor::~FileMonitor()
{
    shouldStop = true;
}

void* FileMonitor::execute(void *args)
{
    socket.connect();
    fd = inotify_init();
    if (fd < 0)
        std::cerr << "Error initializing inotify" << std::endl;

    int wd = inotify_add_watch(fd, monitoredFolder.c_str(), IN_FILE_CHANGE | IN_FILE_REMOVE);
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
        char *msg_buffer;
        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                if (event->mask & (IN_FILE_CHANGE | IN_FILE_REMOVE))
                {
                    if (file_exists(event->name))
                    {
                        if (event->mask & IN_ISDIR)
                        {
                            std::cout << "Directory " << event->name << " modified." << std::endl;
                            msg_buffer = FileMonitorProtocol::encode(FileMonitorProtocol::FMP_FILE_CHANGE, event->name);
                            socket.write(msg_buffer);
                            delete msg_buffer;
                        }
                        else
                        {
                            std::cout << "File " << event->name << " modified." << std::endl;
                            msg_buffer = FileMonitorProtocol::encode(FileMonitorProtocol::FMP_FILE_CHANGE, event->name);
                            socket.write(msg_buffer);
                            delete msg_buffer;
                        }
                    }
                    else
                    {
                        if (event->mask & IN_ISDIR)
                        {
                            std::cout << "Directory " << event->name << " deleted." << std::endl;
                            msg_buffer = FileMonitorProtocol::encode(FileMonitorProtocol::FMP_FILE_REMOVE, event->name);
                            socket.write(msg_buffer);
                            delete msg_buffer;
                        }
                        else
                        {
                            std::cout << "File " << event->name << " deleted." << std::endl;
                            msg_buffer = FileMonitorProtocol::encode(FileMonitorProtocol::FMP_FILE_REMOVE, event->name);
                            socket.write(msg_buffer);
                            delete msg_buffer;
                        }
                    }
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

    inotify_rm_watch(fd, watchedFolder);

    close(fd);

    return NULL;
}

void FileMonitor::stop()
{
    inotify_rm_watch(fd, watchedFolder);
    close(fd);
    shouldStop = true;
    Thread::stop();
}