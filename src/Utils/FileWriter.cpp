#include "Utils/FileWriter.hpp"

FileWriter::FileWriter(const std::string user, const unsigned int clientId, Mutex* mutex, const std::string filename, const int socketPort) : FileHandler(user, clientId, mutex, filename, new SocketClient("localhost", socketPort), FileHandlerMode::WRITE){}

bool FileWriter::operator==(const FileWriter& other) const
{
    return this == &other;
}