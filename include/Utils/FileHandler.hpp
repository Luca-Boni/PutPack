#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketClient.hpp"
#include "Utils/Mutex.hpp"
#include "Utils/FileHandlerProtocol.hpp"
#include <string>
#include <fstream>

class FileHandler : public Thread
{
private:
    static const std::string dataFolder;
    static unsigned long long lastId;
    const unsigned long long id;
    const unsigned int clientId;
    const FileHandlerMode mode;
    SocketClient* socketClient;
    Mutex* mutex;
    const std::string user;
    const std::string filename;
    std::fstream file;
    void *execute(void *dummy);
    void readFile();
    void writeFile();
    void deleteFile();

public:
    FileHandler();
    FileHandler(const std::string user, const unsigned int clientId, Mutex* mutex, const std::string filename, SocketClient* socketClient, const FileHandlerMode mode = FileHandlerMode::READ);
    ~FileHandler();
    void waitTillConnect();
    void stop();
    static unsigned long long getNewId() {return lastId + 1;};
};