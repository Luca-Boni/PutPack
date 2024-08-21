#pragma once

#include <string>
#include "Utils/Thread.hpp"
#include "Utils/MutexHash.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/FileWriter.hpp"
#include "Utils/FileReader.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Hash.hpp"
#include <unordered_map>
#include <tuple>

class UserManager : public Thread
{
private:
    bool backup;
    std::string username;
    std::vector<std::string> files;
    bool shouldStop;

    SocketServer socketServer;
    SocketServerSession socket;         // Socket to communicate with ClientManager
    SocketClient socketClient;          // Sent to ClientManagers and FileReaders to communicate with this thread

    MutexHash<std::string> fileMutexes; // Mutexes to protect the files

    std::unordered_map<unsigned long long, std::unordered_map<unsigned long long, FileReader *>> fileReaders; // Chave de fora = clientId, chave do map = fileHandlerId
    std::unordered_map<unsigned long long, std::unordered_map<unsigned long long, FileWriter *>> fileWriters;
    std::unordered_map<unsigned long long, SocketServerSession *> fileWriterSessions;
    std::unordered_map<unsigned long long, bool> isSyncing;

    // std::unordered_map<unsigned long long, FileReader *> fileReaders;
    // std::unordered_map<unsigned long long, FileWriter *> fileWriters;

    std::unordered_map<unsigned long long, SocketServerSession *> clientManagerSockets;

    void *execute(void *dummy);
    void processFileWriteMsg(const char *buffer);
    void processFileReadMsg(const char *buffer);
    void processNewClientMsg(const char *buffer);
    void processEndClientMsg(const char *buffer);
    void processSyncAllMsg(const char *buffer);
    void processFileDeleteMsg(const char *buffer);
    void processFileDownloadMsg(const char *buffer);
    void processListServerFilesMsg(const char *buffer);

    void processSetToPrimaryMsg(const char *buffer);
    
    void readAllFiles(unsigned long long clientId);

public:
    UserManager(const std::string username, bool backup);
    ~UserManager(){};
    SocketClient *getSocketClient() { return &socketClient; };
    void stopGraciously();
    void stop();

    void setClientManagerSockets(std::unordered_map<unsigned long long, SocketServerSession *> clientManagerSockets);

    void setToPrimary();
};