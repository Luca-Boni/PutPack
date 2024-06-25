#include "Server/FileHandler.hpp"
#include "Server/FileHandlerProtocol.hpp"
#include <fstream>
#include <iostream>
#include <cstring>

const std::string FileHandler::dataFolder = "data/";

FileHandler::FileHandler() {}

FileHandler::FileHandler(const std::string user, Mutex mutex, const std::string filename, const int serverPort, const FileHandlerMode mode, const std::string hostAddress) : Thread(std::bind(&FileHandler::execute, this, std::placeholders::_1), NULL),
                                                                                                                                                                             user(user),
                                                                                                                                                                             filename(filename),
                                                                                                                                                                             mutex(mutex),
                                                                                                                                                                             mode(mode)
{
    socketClient = SocketClient(hostAddress.c_str(), serverPort);
}

FileHandler::~FileHandler() {}

void *FileHandler::execute(void *dummy)
{
    socketClient.connect();
    mutex.lock();
    if (mode == FileHandlerMode::READ)
        readFile();
    else
        writeFile();
    mutex.unlock();
    return NULL;
}

void FileHandler::readFile()
{
    std::ifstream file(dataFolder + user + "/" + filename, std::ios_base::binary);

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

        char *msg_buffer = FileHandlerProtocol::encode(filename.c_str(), buffer, bytesRead);
        socketClient.write(msg_buffer);
        delete msg_buffer;

        delete buffer;
    } while (!done);

    char *msg_buffer = FileHandlerProtocol::encode(filename.c_str(), NULL, 0);
    socketClient.write(msg_buffer);
    delete msg_buffer;
}

void FileHandler::writeFile()
{
    std::ofstream file(dataFolder + user + "/" + filename, std::ios_base::binary | std::ios_base::trunc);
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
            char *buffer = new char[SOCKET_BUFFER_SIZE]();
            socketClient.read(buffer);
            size = *(int *)(buffer+sizeof(char));
            char *data = &buffer[sizeof(char) + sizeof(int) + FILENAME_SIZE];
            file.write(data, size);
            delete buffer;
        } while (size > 0);
    }
}