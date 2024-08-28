#include "Utils/FileReader.hpp"

FileReader::FileReader(const std::string user, const unsigned long long clientId, Mutex* mutex, const std::string filename, SocketClient* socketClient, std::string customPath) : FileHandler(user, clientId, mutex, filename, socketClient, FileHandlerMode::READ, customPath){}