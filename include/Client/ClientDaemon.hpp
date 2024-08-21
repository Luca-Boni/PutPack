#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/SocketClient.hpp"
#include "Utils/FileReader.hpp"
#include "Utils/FileWriter.hpp"
#include "Utils/MutexHash.hpp"
#include "Client/FileMonitor.hpp"
#include "Client/FrontEnd.hpp"
#include "Client/ServerReceiver.hpp"

#include <string>
#include <unordered_set>

class ClientDaemon : public Thread
{
private:
    const std::string username;
    bool shouldStop;
    bool isConnected;
    bool isSyncing;
    std::string syncDir;

    FileMonitor *fileMonitor;

    SocketServer socket;
    SocketServerSession *socketSession; // Socket to read messages from ServerReceiver, FileReaders and FileMonitor
    SocketClient socketClient;          // Socket to ServerReceiver, FileReaders and FileWriters write to
    SocketClient *serverSocket;         // Socket to communicate with the server

    SocketServer confSocket;
    SocketServerSession confirmationSocket; // Socket to receive OK message from server
    SocketClient cliConfSocket;

    std::unordered_set<std::string> filesBeingRead;
    std::unordered_set<std::string> filesBeingWritten;

    MutexHash<std::string> fileMutexes; // Mutexes to protect the files

    std::unordered_map<std::string, FileReader *> fileReaders;
    std::unordered_map<std::string, FileWriter *> fileWriters;
    std::unordered_map<std::string, SocketServerSession *> fileWriterSessions;

    std::vector<char *> pendingMessages;

    FrontEnd *frontEnd;
    int frontEndPort;

    ServerReceiver *serverReceiver;

    void *execute(void *dummy);

    void endClient();

    void processConnectionRejectedMsg(const char *buffer);
    void processFileMonitorMsg(const char *buffer);
    void processFileReadMsg(const char *buffer);
    void processFileWriteMsg(const char *buffer);
    void processFileDeleteMsg(const char *buffer);
    void processNewServerMsg(const char *buffer);

    void processFileUploadMsg(const char *buffer);

    void processInterfaceCommand(const char *buffer);
    void processListServerFilesMsg(const char *buffer);

    void uploadFile(const std::string &filepath);
    void downloadFile(const std::string filename);
    void deleteFile(const std::string &filename);

    void listServerFiles();
    void listClientFiles();
    void synchronize();

public:
    ClientDaemon(const std::string &username, SocketClient *serverSocket, ServerReceiver *serverReceiver);
    ClientDaemon() {};
    ~ClientDaemon() {};

    SocketClient *getSocketClient() { return &socketClient; }
    void setServerSocket(SocketClient *serverSocket) { this->serverSocket = serverSocket; }

    SocketClient *getCliConfSocket() { return &cliConfSocket; }
};