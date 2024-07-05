#include <iostream>
#include "Utils/SocketClient.hpp"
#include "Utils/Protocol.hpp"
#include "SyncManager.hpp"

void printUsage() {
    std::cout << "Usage: ./myClient <username> <server_ip_address> <port>\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage();
        return 1;
    }

    std::string username = argv[1];
    std::string serverIp = argv[2];
    int port = std::stoi(argv[3]);

    SyncManager syncManager("sync_dir_" + username, username, serverIp, port);
    syncManager.start();

    std::string command;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        if (command == "exit") {
            break;
        } else if (command.find("upload ") == 0) {
            // Implement upload functionality
        } else if (command.find("download ") == 0) {
            // Implement download functionality
        } else if (command == "list_server") {
            // Implement list_server functionality
        } else if (command == "list_client") {
            // Implement list_client functionality
        } else if (command == "get_sync_dir") {
            // Implement get_sync_dir functionality
        } else {
            std::cout << "Unknown command: " << command << "\n";
        }
    }

    syncManager.stop();
    return 0;
}
