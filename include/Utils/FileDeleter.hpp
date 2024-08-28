#pragma once

#include "Utils/FileHandler.hpp"

class FileDeleter : public FileHandler
{
public:
    FileDeleter(const std::string user, const unsigned long long clientId, Mutex* mutex, const std::string filename);
    ~FileDeleter(){};
};