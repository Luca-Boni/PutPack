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
    static unsigned long long lastId;
    const unsigned long long id;
    const unsigned long long clientId;
    const FileHandlerMode mode;
    SocketClient* socketClient;
    Mutex* mutex;
    const std::string user;
    const std::string filename;
    std::fstream file;
    std::string customPath;

    void *execute(void *dummy);
    void readFile();
    void writeFile();
    void deleteFile();

public:
    FileHandler();
    FileHandler(const std::string user, const unsigned long long clientId, Mutex* mutex, const std::string filename, SocketClient* socketClient, const FileHandlerMode mode = FileHandlerMode::READ, std::string customPath = "");
    ~FileHandler();
    void waitTillConnect();
    void stop();
    static unsigned long long getNewId() {return lastId + 1;};
    unsigned long long getId() {return id;};
};