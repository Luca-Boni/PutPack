#include "Utils/FileReader.hpp"

FileReader::FileReader(const std::string user, const unsigned int clientId, Mutex* mutex, const std::string filename, SocketClient* socketClient) : FileHandler(user, clientId, mutex, filename, socketClient, FileHandlerMode::READ){}