#pragma once

#include <string>
#include <unordered_map>
#include "thread.hpp"
#include "socketclient.hpp"
#include "filewriter.hpp"
#include "filereader.hpp"
#include "mutexhash.hpp"
#include "protocol.hpp"

class ClientManager : public Thread {
private:
    std::string username;
    unsigned long long clientId;
    bool shouldStop;

    SocketClient socketClient;  // Socket to communicate with UserManager
    MutexHash<std::string> fileMutexes;

    std::unordered_map<unsigned long long, FileWriter *> fileWriters;
    std::unordered_map<unsigned long long, FileReader *> fileReaders;

    void *execute(void *dummy) override;
    void processFileWriteMsg(const char *buffer);
    void processFileReadMsg(const char *buffer);
    void processSyncAllMsg(const char *buffer);
    void processFileDeleteMsg(const char *buffer);

public:
    ClientManager(const std::string &username, const std::string &serverAddress, int serverPort);
    ~ClientManager();
    void connectToServer();
    void stopGraciously();
    void stop();
    void sendFileWriteMsg(const std::string &filename, const char *data, size_t size);
    void sendFileReadMsg(const std::string &filename);
    void sendSyncAllMsg();
    void sendFileDeleteMsg(const std::string &filename);
};
