#include "SyncManager.hpp"
#include "Utils/SocketClient.hpp"
#include "Utils/FileMonitorProtocol.hpp"
#include <iostream>

SyncManager::SyncManager(const std::string& syncDir, const std::string& username, const std::string& serverIp, int port)
    : fileMonitor(syncDir), username(username), serverIp(serverIp), port(port) {
    fileMonitor.setCallback([this](const std::string& filename, FileMonitor::Event event) {
        if (event == FileMonitor::Event::MODIFIED) {
            onFileChange(filename);
        } else if (event == FileMonitor::Event::DELETED) {
            onFileDelete(filename);
        }
    });
}

void SyncManager::start() {
    fileMonitor.start();
}

void SyncManager::stop() {
    fileMonitor.stop();
}

void SyncManager::onFileChange(const std::string& filename) {
    SocketClient socket(serverIp.c_str(), port);
    socket.connect();
    char* message = FileMonitorProtocol::encode(FileMonitorProtocol::FMP_FILE_CHANGE, filename.c_str());
    socket.write(message);
    delete[] message;
    socket.close();
}

void SyncManager::onFileDelete(const std::string& filename) {
    SocketClient socket(serverIp.c_str(), port);
    socket.connect();
    char* message = FileMonitorProtocol::encode(FileMonitorProtocol::FMP_FILE_REMOVE, filename.c_str());
    socket.write(message);
    delete[] message;
    socket.close();
}
