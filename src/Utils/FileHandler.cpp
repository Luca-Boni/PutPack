#include "Utils/FileHandler.hpp"
#include "Utils/FileHandlerProtocol.hpp"
#include "Utils/Protocol.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <unistd.h>

unsigned long long FileHandler::lastId = 0;

std::string getFileFolder(std::string username)
{
    char path[FILENAME_MAX];
    ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
    std::string exe_dir = std::filesystem::path(std::string(path)).parent_path().string();
    return exe_dir + "/sync_dir_" + username + "/";
}

FileHandler::FileHandler() : id(getNewId()),
                             user(""),
                             clientId(0),
                             filename(""),
                             mode(FileHandlerMode::READ) {}

FileHandler::FileHandler(const std::string user, const unsigned long long clientId, Mutex *mutex, const std::string filename, SocketClient *socketClient, const FileHandlerMode mode, std::string customPath) : Thread(std::bind(&FileHandler::execute, this, std::placeholders::_1), NULL),
                                                                                                                                                                                                                id(getNewId()),
                                                                                                                                                                                                                user(user),
                                                                                                                                                                                                                clientId(clientId),
                                                                                                                                                                                                                filename(filename),
                                                                                                                                                                                                                mutex(mutex),
                                                                                                                                                                                                                socketClient(socketClient),
                                                                                                                                                                                                                mode(mode),
                                                                                                                                                                                                                customPath(customPath) {}

FileHandler::~FileHandler() {}

void *FileHandler::execute(void *dummy)
{
    if (socketClient != NULL)
        socketClient->connect();
    mutex->lock();
    switch (mode)
    {
    case FileHandlerMode::READ:
        readFile();
        break;
    case FileHandlerMode::WRITE:
        writeFile();
        break;
    case FileHandlerMode::DELETE:
        deleteFile();
        break;
    default:
        std::cerr << "Unknown mode" << std::endl;
        break;
    }
    mutex->unlock();
    return NULL;
}

void FileHandler::readFile()
{
    bool isUpload = !(customPath.empty());
    bool isDownload = (filename.find("./") == 0);

    std::cout << customPath << std::endl;

    std::string fullFilename;
    if (isDownload)
    {
        fullFilename = getFileFolder(user) + filenameFromPath(filename);
    }
    else if (isUpload)
    {
        fullFilename = customPath;
    }
    else
    {
        fullFilename = getFileFolder(user) + filename;
    }

    file = std::fstream(fullFilename, std::fstream::in | std::fstream::binary);

    if (!file.is_open())
    {
        std::cerr << "Error opening file '" << fullFilename << "'." << std::endl;
        return;
    }

    bool done = false;

    do
    {
        char *buffer = new char[FILE_BUFFER_SIZE]();

        file.read(buffer, FILE_BUFFER_SIZE);
        std::streamsize bytesRead = file.gcount();
        done = (file.peek() == EOF);

        struct FileHandlerMessage msg = FileHandlerMessage(clientId, filename.c_str(), bytesRead, buffer);
        char *msg_buffer = msg.encode();
        if (isUpload)
            msg_buffer[0] = FILE_UPLOAD_MSG;
        socketClient->write(msg_buffer);
        delete[] msg_buffer;

        delete[] buffer;
    } while (!done);

    struct FileHandlerMessage msg = FileHandlerMessage(clientId, filename.c_str(), 0, NULL);
    char *msg_buffer = msg.encode();
    if (isUpload)
        msg_buffer[0] = FILE_UPLOAD_MSG;
    socketClient->write(msg_buffer);
    delete[] msg_buffer;

    file.close();
}

void FileHandler::writeFile()
{
    bool isDownload = (filename.find("./") == 0);
    std::string filename = isDownload ? this->filename : getFileFolder(user) + this->filename;

    file = std::fstream(filename, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    int size;

    if (!file.is_open())
    {
        std::cerr << "Error opening file '" << filename << "'." << std::endl;
        return;
    }
    else
    {
        do
        {
            char *buffer = socketClient->read();
            struct FileHandlerMessage msg;
            msg.decode(buffer);
            size = msg.size;
            file.write(msg.data, size);
            delete[] buffer;
        } while (size > 0);
    }

    file.close();
}

void FileHandler::deleteFile()
{
    std::remove((getFileFolder(user) + filename).c_str());
}

void FileHandler::stop()
{
    Thread::stop();
    file.close();
    mutex->unlock();
}

void FileHandler::waitTillConnect()
{
    while (!socketClient->isConnected() && !(socketClient->getErrors() && SocketClientError::CANT_CONNECT))
        ;
}