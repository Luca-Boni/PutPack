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

void printUsageAndExit(std::string programName)
{
    std::cerr << "Usage: " << programName << " <ID> [-b <Primary server address:port>] [Server port]" << std::endl;
    exit(1);
}

int main(int argc, char *argv[])
{

    int connectionManagerPort = 0;
    bool backup = false;
    std::string primaryAddress = "";
    int id = 0;

    if (argc == 3)
    {
        connectionManagerPort = atoi(argv[2]);
        if (connectionManagerPort == 0)
        {
            printUsageAndExit(argv[0]);
        }
    }
    else if (argc == 4)
    {
        if (strcmp(argv[2], "-b") == 0)
        {
            backup = true;
            primaryAddress = argv[3];
        }
        else
        {
            printUsageAndExit(argv[0]);
        }
    }
    else if (argc == 5)
    {
        if (strcmp(argv[2], "-b") == 0)
        {
            backup = true;
            primaryAddress = argv[3];
            connectionManagerPort = atoi(argv[4]);
            if (connectionManagerPort == 0)
            {
                printUsageAndExit(argv[0]);
            }
        }
        else
        {
            printUsageAndExit(argv[0]);
        }
    }
    else if (argc > 6 || argc < 2)
    {
        printUsageAndExit(argv[0]);
    }

    if (argc <= 5 && argc >= 2)
    {
        id = atoi(argv[1]);
        Logger logger = Logger("PutPackServer");

        ServerDaemon serverDaemon = ServerDaemon(id, backup, primaryAddress);
        serverDaemon.start();
        SocketClient *serverDaemonClientSocket = serverDaemon.getSocketClient();
        serverDaemonClientSocket->connect();

        ConnectionManager connectionManager = ConnectionManager(connectionManagerPort, serverDaemonClientSocket);
        serverDaemon.setPort(connectionManager.getPort());

        connectionManager.start();
        
        // serverDaemonPtr = &serverDaemon;
        // connectionManagerPtr = &connectionManager;
        // loggerPtr = &logger;
        // signal(SIGINT, ctrlCHandler);

        std::string asBackup = backup ? "as backup " : "";

        std::cout << "Server started " << asBackup << "on port " << connectionManager.getPort() << std::endl
                  << "Type 'exit' to stop the server." << std::endl
                  /*<< "[NOT RECOMMENDED] Ctrl+C will kill the server immediately." << std::endl*/;

        Logger::log("Server started " + asBackup + "on port " + std::to_string(connectionManager.getPort()));

        std::string command;
        while (command != "exit")
        {
            std::cin >> command;
            if (command == "primary")
            {
                serverDaemon.setToPrimary();
            }
        }

        connectionManager.stop();
        serverDaemon.stopGraciously();

        Logger::log("Waiting for threads to join...");

        connectionManager.join();
        serverDaemon.join();
        logger.stop();

        Logger::log("Server closed successfully.");
    }

    return 0;
}