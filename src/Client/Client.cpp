#include "Client.hpp"
#include <iostream>
#include <fstream>

Client::Client(const std::string& serverAddress, int serverPort) 
    : socketClient(serverAddress, serverPort), isConnected(false) {}

Client::~Client() {
    disconnectFromServer();
}

bool Client::connectToServer() {
    if (socketClient.connect()) {
        isConnected = true;
        std::cout << "Connected to server." << std::endl;
        return true;
    } else {
        std::cerr << "Failed to connect to server." << std::endl;
        return false;
    }
}

void Client::disconnectFromServer() {
    if (isConnected) {
        socketClient.close();
        isConnected = false;
        std::cout << "Disconnected from server." << std::endl;
    }
}

bool Client::isConnectedToServer() const {
    return isConnected;
}

void Client::upload(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Prepare command
    std::string command = "UPLOAD " + std::to_string(fileSize) + " " + filePath;
    sendCommand(command);

    // Send file content
    char* buffer = new char[fileSize];
    file.read(buffer, fileSize);
    socketClient.write(buffer, fileSize);
    delete[] buffer;

    // Receive response
    std::string response = receiveResponse();
    std::cout << "Server response: " << response << std::endl;

    file.close();
}

void Client::download(const std::string& fileName) {
    // Prepare command
    std::string command = "DOWNLOAD " + fileName;
    sendCommand(command);

    // Receive file size
    std::string sizeStr = receiveResponse();
    if (sizeStr == "FILE_NOT_FOUND") {
        std::cerr << "File not found on server: " << fileName << std::endl;
        return;
    }

    std::size_t fileSize = std::stoul(sizeStr);
    std::cout << "Downloading file " << fileName << " with size " << fileSize << " bytes." << std::endl;

    // Receive file content
    char* buffer = new char[fileSize];
    socketClient.read(buffer, fileSize);

    // Save to file
    std::ofstream file(fileName, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to create file: " << fileName << std::endl;
        delete[] buffer;
        return;
    }
    file.write(buffer, fileSize);
    file.close();

    delete[] buffer;
    std::cout << "File downloaded successfully." << std::endl;
}

void Client::deleteFile(const std::string& fileName) {
    // Prepare command
    std::string command = "DELETE " + fileName;
    sendCommand(command);

    // Receive response
    std::string response = receiveResponse();
    std::cout << "Server response: " << response << std::endl;
}

void Client::listServerFiles() {
    // Prepare command
    std::string command = "LIST_SERVER";
    sendCommand(command);

    // Receive response
    std::string response = receiveResponse();
    std::cout << "Server files: " << response << std::endl;
}

void Client::listClientFiles() {
    // Prepare command
    std::string command = "LIST_CLIENT";
    sendCommand(command);

    // Receive response
    std::string response = receiveResponse();
    std::cout << "Client files: " << response << std::endl;
}

void Client::synchronize() {
    // Prepare command
    std::string command = "SYNC";
    sendCommand(command);

    // Receive response
    std::string response = receiveResponse();
    std::cout << "Sync response: " << response << std::endl;
}

void Client::sendCommand(const std::string& command) {
    socketClient.write(command.c_str(), command.length());
}

std::string Client::receiveResponse() {
    char buffer[1024];
    int bytesRead = socketClient.read(buffer, sizeof(buffer) - 1);
    buffer[bytesRead] = '\0';
    return std::string(buffer);
}
