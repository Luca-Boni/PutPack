#include "Utils/FileHandler.hpp"
#include "Utils/FileHandlerProtocol.hpp"
#include <fstream>
#include <iostream>
#include <cstring>

unsigned long long FileHandler::lastId = 0;

const std::string FileHandler::dataFolder = "data/";

FileHandler::FileHandler() : id(getNewId()),
                             user(""),
                             clientId(0),
                             filename(""),
                             mode(FileHandlerMode::READ) {}

FileHandler::FileHandler(const std::string user, const unsigned int clientId, Mutex *mutex, const std::string filename, SocketClient *socketClient, const FileHandlerMode mode) : Thread(std::bind(&FileHandler::execute, this, std::placeholders::_1), NULL),
                                                                                                                                                     id(getNewId()),
                                                                                                                                                     user(user),
                                                                                                                                                     clientId(clientId),
                                                                                                                                                     filename(filename),
                                                                                                                                                     mutex(mutex),
                                                                                                                                                     socketClient(socketClient),
                                                                                                                                                     mode(mode) {}

FileHandler::~FileHandler() {}

void *FileHandler::execute(void *dummy)
{
    if(socketClient != NULL)
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
    file = std::fstream(dataFolder + user + "/" + filename, std::fstream::in | std::fstream::binary);

    if (!file.is_open())
    {
        std::cerr << "Error opening file " << filename << std::endl;
        return;
    }

    bool done = false;

    do
    {
        char *buffer = new char[FILE_BUFFER_SIZE]();

        file.read(buffer, FILE_BUFFER_SIZE);
        std::streamsize bytesRead = file.gcount();
        done = (file.peek() == EOF);

        struct FileHandlerMessage msg = FileHandlerMessage(id, clientId, filename.c_str(), bytesRead, buffer);
        char *msg_buffer = msg.encode();
        socketClient->write(msg_buffer);
        delete[] msg_buffer;

        delete[] buffer;
    } while (!done);

    struct FileHandlerMessage msg = FileHandlerMessage(id, clientId, filename.c_str(), 0, NULL);
    char *msg_buffer = msg.encode();
    socketClient->write(msg_buffer);
    delete[] msg_buffer;
}

void FileHandler::writeFile()
{
    file = std::fstream(dataFolder + user + "/" + filename, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    int size;

    if (!file.is_open())
    {
        std::cerr << "Error opening file " << filename << std::endl;
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
}

void FileHandler::deleteFile()
{
    std::remove((dataFolder + user + "/" + filename).c_str());
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