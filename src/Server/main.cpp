#include "Server/ServerDaemon.hpp"
#include "Server/ConnectionManager.hpp"
#include "Utils/Logger.hpp"

#include <iostream>
#include <signal.h>

ServerDaemon *serverDaemonPtr = NULL;
ConnectionManager *connectionManagerPtr = NULL;
Logger *loggerPtr = NULL;

void ctrlCHandler(sig_atomic_t s)
{
    if (connectionManagerPtr)
        connectionManagerPtr->stop();
    if (serverDaemonPtr)
        serverDaemonPtr->stop();
    if (loggerPtr)
        loggerPtr->stop();
    std::cout << "Server stopped forcefully." << std::endl;
    exit(2);
}

int main(int argc, char *argv[])
{

    int connectionManagerPort = 0;

    if (argc == 2)
        connectionManagerPort = atoi(argv[1]);
    else if (argc > 2)
    {
        std::cerr << "Usage: " << argv[0] << " [Server port]" << std::endl;
        return 1;
    }

    if (argc <= 2)
    {
        Logger logger = Logger("PutPackServer");

        ServerDaemon serverDaemon = ServerDaemon();
        serverDaemon.start();
        SocketClient *serverDaemonClientSocket = serverDaemon.getSocketClient();
        serverDaemonClientSocket->connect();

        ConnectionManager connectionManager = ConnectionManager(connectionManagerPort, serverDaemonClientSocket);
        connectionManager.start();
        
        serverDaemonPtr = &serverDaemon;
        connectionManagerPtr = &connectionManager;
        loggerPtr = &logger;
        signal(SIGINT, ctrlCHandler);        

        std::cout << "Server started on port " << connectionManager.getPort() << std::endl
                  << "Type 'exit' to stop the server." << std::endl
                  << "[NOT RECOMMENDED] Ctrl+C will kill the server immediately." << std::endl;

        Logger::log("Server started on port " + std::to_string(connectionManager.getPort()));

        std::string command;
        while (command != "exit")
            std::cin >> command;

        connectionManager.stop();
        serverDaemon.stopGraciously();

        connectionManager.join();
        serverDaemon.join();
    }

    return 0;
}