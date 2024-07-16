#include "Client/FileMonitor.hpp"
#include <iostream>
#include <sys/stat.h>

bool FileMonitor::file_exists(const std::string filename)
{
    struct stat buffer;
    return (stat((monitoredFolder + "/" + filename).c_str(), &buffer) == 0);
}

FileMonitor::FileMonitor() {}

FileMonitor::FileMonitor(const std::string monitoredFolder, SocketClient *socketClient) : Thread(std::bind(&FileMonitor::execute, this, std::placeholders::_1), NULL)
{
    socket = socketClient;
    this->monitoredFolder = monitoredFolder;
    shouldStop = false;
    disabledFiles = std::unordered_set<std::string>();
}

FileMonitor::~FileMonitor()
{
    shouldStop = true;
}

void *FileMonitor::execute(void *dummy)
{
    socket->connect();
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
                            // std::cout << "Directory " << event->name << " modified." << std::endl;
                            // FileMonitorMsg msg(FileMonitorProtocol::FMP_FILE_CHANGE, event->name);
                            // msg_buffer = msg.encode();
                            // socket.write(msg_buffer);
                            // delete[] msg_buffer;
                        }
                        else
                        {
                            if (disabledFiles.find(event->name) != disabledFiles.end())
                            {
                                // std::cout << "File " << event->name << " is disabled." << std::endl;
                            }
                            else
                            {
                                // std::cout << "File " << event->name << " modified." << std::endl;
                                FileMonitorMsg msg(FileMonitorProtocol::FMP_FILE_CHANGE, event->name);
                                msg_buffer = msg.encode();
                                socket->write(msg_buffer);
                                delete[] msg_buffer;
                            }
                        }
                    }
                    else
                    {
                        if (event->mask & IN_ISDIR)
                        {
                            // std::cout << "Directory " << event->name << " deleted." << std::endl;
                            // FileMonitorMsg msg(FileMonitorProtocol::FMP_FILE_REMOVE, event->name);
                            // msg_buffer = msg.encode();
                            // socket.write(msg_buffer);
                            // delete[] msg_buffer;
                        }
                        else
                        {
                            if (disabledFiles.find(event->name) != disabledFiles.end())
                            {
                                // std::cout << "File " << event->name << " is disabled." << std::endl;
                            }
                            else
                            {
                                // std::cout << "File " << event->name << " deleted." << std::endl;
                                FileMonitorMsg msg(FileMonitorProtocol::FMP_FILE_REMOVE, event->name);
                                msg_buffer = msg.encode();
                                socket->write(msg_buffer);
                                delete[] msg_buffer;
                            }
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

void FileMonitor::disableFile(const std::string filename)
{
    disabledFiles.insert(filename);
}

void FileMonitor::enableFile(const std::string filename)
{
    disabledFiles.erase(filename);
}