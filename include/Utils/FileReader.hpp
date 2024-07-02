#pragma once

#include "Utils/FileHandler.hpp"

class FileReader : public FileHandler
{
public:
    FileReader(const std::string user, const unsigned int clientId, Mutex* mutex, const std::string filename, SocketClient* socketclient);
    ~FileReader(){};
};