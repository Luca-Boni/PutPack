#include "ClientManager.hpp"
#include <iostream>
#include <cstring>

ClientManager::ClientManager(const std::string &username, const std::string &serverAddress, int serverPort)
    : Thread(std::bind(&ClientManager::execute, this, std::placeholders::_1), nullptr),
      username(username),
      clientId(0), // Assign a unique client ID as needed
      shouldStop(false),
      socketClient(serverAddress, serverPort) {
}

ClientManager::~ClientManager() {
    stop();
}

void *ClientManager::execute(void *dummy) {
    try {
        connectToServer();
        
        while (!shouldStop) {
            char *buffer = socketClient.read();
            switch (buffer[0]) {
                case FILE_WRITE_MSG:
                    processFileWriteMsg(buffer);
                    break;
                case FILE_READ_MSG:
                    processFileReadMsg(buffer);
                    break;
                case SYNC_MSG:
                    processSyncAllMsg(buffer);
                    break;
                case FILE_DELETE_MSG:
                    processFileDeleteMsg(buffer);
                    break;
                default:
                    std::cerr << "Unknown message received: " << +buffer[0] << std::endl;
                    break;
            }
            delete[] buffer;
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception in ClientManager::execute: " << e.what() << std::endl;
    }
    return nullptr;
}

void ClientManager::connectToServer() {
    socketClient.connect();
    NewClientMsg msg;
    msg.clientId = clientId;
    msg.socketClient = &socketClient;
    char *encodedMsg = msg.encode();
    socketClient.write(encodedMsg);
    delete[] encodedMsg;
}

void ClientManager::processFileWriteMsg(const char *buffer) {
    FileHandlerMessage msg;
    msg.decode(buffer);
    if (fileWriters.find(msg.fileHandlerId) == fileWriters.end()) {
        FileWriter *writer = new FileWriter(username, msg.clientId, fileMutexes.getOrAddMutex(msg.filename), msg.filename, socketClient.getPort());
        fileWriters[msg.fileHandlerId] = writer;
        writer->start();
    }
    fileWriters[msg.fileHandlerId]->write(buffer, msg.size);
    if (msg.size == 0) {
        fileWriters[msg.fileHandlerId]->join();
        delete fileWriters[msg.fileHandlerId];
        fileWriters.erase(msg.fileHandlerId);
    }
}

void ClientManager::processFileReadMsg(const char *buffer) {
    FileHandlerMessage msg;
    msg.decode(buffer);
    if (fileReaders.find(msg.fileHandlerId) == fileReaders.end()) {
        FileReader *reader = new FileReader(username, msg.clientId, fileMutexes.getOrAddMutex(msg.filename), msg.filename, &socketClient);
        fileReaders[msg.fileHandlerId] = reader;
        reader->start();
    }
}

void ClientManager::processSyncAllMsg(const char *buffer) {
    SyncAllMsg msg;
    msg.decode(buffer);
    // Implement sync logic here
}

void ClientManager::processFileDeleteMsg(const char *buffer) {
    FileHandlerMessage msg;
    msg.decode(buffer);
    // Implement delete logic here
}

void ClientManager::stopGraciously() {
    shouldStop = true;
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    buffer[0] = STOP_MSG;
    socketClient.write(buffer);
    delete[] buffer;
}

void ClientManager::stop() {
    shouldStop = true;
    socketClient.close();
    Thread::stop();
}

void ClientManager::sendFileWriteMsg(const std::string &filename, const char *data, size_t size) {
    FileHandlerMessage msg;
    msg.clientId = clientId;
    msg.filename = filename;
    msg.size = size;
    char *buffer = msg.encode(data, size);
    socketClient.write(buffer);
    delete[] buffer;
}

void ClientManager::sendFileReadMsg(const std::string &filename) {
    FileHandlerMessage msg;
    msg.clientId = clientId;
    msg.filename = filename;
    msg.size = 0;
    char *buffer = msg.encode();
    socketClient.write(buffer);
    delete[] buffer;
}

void ClientManager::sendSyncAllMsg() {
    SyncAllMsg msg;
    msg.clientId = clientId;
    char *buffer = msg.encode();
    socketClient.write(buffer);
    delete[] buffer;
}

void ClientManager::sendFileDeleteMsg(const std::string &filename) {
    FileHandlerMessage msg;
    msg.clientId = clientId;
    msg.filename = filename;
    msg.size = 0;
    char *buffer = msg.encode();
    socketClient.write(buffer);
    delete[] buffer;
}
