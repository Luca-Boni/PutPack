#include "Utils/FileWriter.hpp"

FileWriter::FileWriter(const std::string user, const unsigned long long clientId, Mutex* mutex, const std::string filename, const int socketPort, std::string customPath) : FileHandler(user, clientId, mutex, filename, new SocketClient("localhost", socketPort), FileHandlerMode::WRITE, customPath){}

bool FileWriter::operator==(const FileWriter& other) const
{
    return this == &other;
}