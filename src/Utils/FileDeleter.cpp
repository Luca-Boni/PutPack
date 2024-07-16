#include "Utils/FileDeleter.hpp"

FileDeleter::FileDeleter(const std::string user, const unsigned long long clientId, Mutex* mutex, const std::string filename) : FileHandler(user, clientId, mutex, filename, NULL, FileHandlerMode::DELETE){}