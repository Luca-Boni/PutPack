#include "Utils/FileDeleter.hpp"

FileDeleter::FileDeleter(const std::string user, const unsigned int clientId, Mutex* mutex, const std::string filename) : FileHandler(user, clientId, mutex, filename, NULL, FileHandlerMode::DELETE){}