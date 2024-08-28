#pragma once

#include "Utils/FileHandler.hpp"

class FileReader : public FileHandler
{
public:
    FileReader(const std::string user, const unsigned long long clientId, Mutex* mutex, const std::string filename, SocketClient* socketclient, std::string customPath = "");
    ~FileReader(){};
};