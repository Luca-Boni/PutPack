#pragma once

#include "Utils/FileHandler.hpp"

class FileWriter : public FileHandler
{
public:
    FileWriter() : FileHandler(){};
    FileWriter(const std::string user, const unsigned long long clientId, Mutex* mutex, const std::string filename, const int socketPort, std::string customPath = "");
    ~FileWriter(){};
    bool operator==(const FileWriter& other) const;
};