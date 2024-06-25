#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketClient.hpp"
#include "Utils/Mutex.hpp"
#include "Server/FileHandlerProtocol.hpp"
#include <string>
#include <fstream>

class FileHandler : public Thread
{
private:
    static const std::string dataFolder;
    FileHandlerMode mode;
    SocketClient socketClient;
    Mutex mutex;
    const std::string user;
    const std::string filename;
    void *execute(void *dummy);
    void readFile();
    void writeFile();

public:
    FileHandler();
    FileHandler(const std::string user, Mutex mutex, const std::string filename, const int serverPort, const FileHandlerMode mode = FileHandlerMode::READ, const std::string hostAddress = "localhost");
    ~FileHandler();
    void stop();
};