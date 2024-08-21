#include "Client/ClientDaemon.hpp"
#include "Client/ServerReceiver.hpp"
#include "Client/ClientMenu.hpp"
#include "Utils/Logger.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << "<username> <server_address> <server_port>" << std::endl;
        exit(1);
    }

    Logger logger = Logger("PutPackClient");
    
    std::string username = argv[1];
    std::string serverAddress = argv[2];
    int serverPort = atoi(argv[3]);

    ServerReceiver serverReceiver = ServerReceiver(serverAddress, serverPort);
    SocketClient *serverSocket = serverReceiver.getServerSocket();

    ClientDaemon clientDaemon = ClientDaemon(username, serverSocket, &serverReceiver);
    SocketClient *clientSocket = clientDaemon.getSocketClient();
    SocketClient *confSocket = clientDaemon.getCliConfSocket();

    serverReceiver.setClientSocket(clientSocket);
    serverReceiver.setConfSocket(confSocket);

    serverReceiver.start();
    clientDaemon.start();
    clientSocket->connect();
    confSocket->connect();

    ClientMenu clientMenu = ClientMenu(clientSocket);
    clientMenu.start();

    serverReceiver.stop();
    clientDaemon.join();
    logger.stop();

    std::cout << "Client stopped sucessfully." << std::endl;

    return 0;
}