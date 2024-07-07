#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SOCKET_BUFFER_SIZE 1024
#define SERVER_IP "192.168.0.186"
#define SERVER_PORT 8080

bool send_data(int socket_fd, const std::string& data) {
    char buffer[SOCKET_BUFFER_SIZE];
    strncpy(buffer, data.c_str(), SOCKET_BUFFER_SIZE);
    return send(socket_fd, buffer, strlen(buffer), 0) >= 0;
}


std::string receive_data(int socket_fd) {
    char buffer[SOCKET_BUFFER_SIZE] = {0};
    recv(socket_fd, buffer, SOCKET_BUFFER_SIZE, 0);
    return std::string(buffer);
}


bool upload_file(int socket_fd, const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Failed to read file: " << file_path << std::endl;
        return false;
    }

    std::stringstream msg;
    msg << "UPLOAD " << file_path << " " << size << " ";
    msg.write(buffer.data(), size);

    return send_data(socket_fd, msg.str());
}

bool download_file(int socket_fd, const std::string& file_name) {
    std::string msg = "DOWNLOAD " + file_name;
    if (!send_data(socket_fd, msg)) {
        std::cerr << "Failed to send download request." << std::endl;
        return false;
    }

    std::string response = receive_data(socket_fd);
    if (response.empty()) {
        std::cerr << "No response from server." << std::endl;
        return false;
    }

    std::ofstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to create file: " << file_name << std::endl;
        return false;
    }

    file.write(response.c_str(), response.size());
    file.close();

    std::cout << "File downloaded: " << file_name << std::endl;
    return true;
}

bool list_files(int socket_fd) {
    std::string msg = "LIST";
    if (!send_data(socket_fd, msg)) {
        std::cerr << "Failed to send list request." << std::endl;
        return false;
    }

    std::string response = receive_data(socket_fd);
    if (response.empty()) {
        std::cerr << "No response from server." << std::endl;
        return false;
    }

    std::cout << "Files on server:\n" << response << std::endl;
    return true;
}

int main() {
    int socket_fd = 0;
    struct sockaddr_in serv_addr;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr)<=0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return 1;
    }

    if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    // Exemplo de uso
    upload_file(socket_fd, "test.txt");
    download_file(socket_fd, "example.txt");
    list_files(socket_fd);

    close(socket_fd);
    return 0;
}
