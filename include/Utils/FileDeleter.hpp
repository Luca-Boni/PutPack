#pragma once

#include "Utils/FileHandler.hpp"

class FileDeleter : public FileHandler
{
public:
    FileDeleter(const std::string user, const unsigned int clientId, Mutex* mutex, const std::string filename);
    ~FileDeleter(){};
};