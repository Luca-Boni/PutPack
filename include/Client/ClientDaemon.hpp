#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/SocketClient.hpp"
#include "Utils/FileReader.hpp"
#include "Utils/FileWriter.hpp"
#include "Utils/MutexHash.hpp"
#include "Client/FileMonitor.hpp"

#include <string>
#include <unordered_set>

class ClientDaemon : public Thread
{
private:
    const std::string username;
    bool shouldStop;
    bool isConnected;
    std::string syncDir;

    FileMonitor* fileMonitor;

    SocketServer socket;
    SocketServerSession socketSession; // Socket to read messages from ServerReceiver, FileReaders and FileMonitor
    SocketClient socketClient;         // Socket to ServerReceiver, FileReaders and FileWriters write to
    SocketClient *serverSocket;        // Socket to communicate with the server

    std::unordered_set<std::string> filesBeingRead;
    std::unordered_set<std::string> filesBeingWritten;

    MutexHash<std::string> fileMutexes; // Mutexes to protect the files

    std::unordered_map<std::string, FileReader *> fileReaders;
    std::unordered_map<std::string, FileWriter *> fileWriters;
    std::unordered_map<std::string, SocketServerSession *> fileWriterSessions;

    void *execute(void *dummy);

    void endClient();

    void processConnectionRejectedMsg(const char *buffer);
    void processFileMonitorMsg(const char *buffer);
    void processFileReadMsg(const char *buffer);
    void processFileWriteMsg(const char *buffer);
    void processFileDeleteMsg(const char *buffer);

    void processFileUploadMsg(const char *buffer);

    void processInterfaceCommand(const char *buffer);
    void processListServerFilesMsg(const char* buffer);

    void uploadFile(const std::string &filepath);
    void downloadFile(const std::string filename);
    void deleteFile(const std::string &filename);
    void listServerFiles();
    void listClientFiles();
    void synchronize();

public:
    ClientDaemon(const std::string &username, SocketClient *serverSocket);
    ClientDaemon(){};
    ~ClientDaemon(){};

    SocketClient *getSocketClient() { return &socketClient; };
    void setServerSocket(SocketClient *serverSocket) { this->serverSocket = serverSocket; };
};