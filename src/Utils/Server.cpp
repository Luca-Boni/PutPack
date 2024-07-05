#include <iostream>
#include "Utils/SocketServer.hpp"
#include "UserManager.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: ./myServer <port>\n";
        return 1;
    }

    int port = std::stoi(argv[1]);
    SocketServer server(port);

    while (true) {
        socket_t clientSocket = server.listenAndAccept();
        std::thread([clientSocket]() {
            UserManager userManager(clientSocket);
            userManager.execute(nullptr);
        }).detach();
    }

    server.close();
    return 0;
}
