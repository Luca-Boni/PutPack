#pragma once

#include "Utils/FileHandler.hpp"

class FileWriter : public FileHandler
{
public:
    FileWriter() : FileHandler(){};
    FileWriter(const std::string user, const unsigned int clientId, Mutex* mutex, const std::string filename, const int socketPort);
    ~FileWriter(){};
    bool operator==(const FileWriter& other) const;
};