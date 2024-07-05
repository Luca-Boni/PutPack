#pragma once

#include <string>
#include "Utils/FileMonitor.hpp"

class SyncManager {
public:
    SyncManager(const std::string& syncDir, const std::string& username, const std::string& serverIp, int port);
    void start();
    void stop();

private:
    void onFileChange(const std::string& filename);
    void onFileDelete(const std::string& filename);

    FileMonitor fileMonitor;
    std::string username;
    std::string serverIp;
    int port;
};
