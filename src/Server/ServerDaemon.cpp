#include "Server/ServerDaemon.hpp"
#include "Utils/Protocol.hpp"
#include "Server/ClientUserProtocol.hpp"

#include <iostream>

#define MAX_CLIENTS_PER_USER 2

ServerDaemon::ServerDaemon() : Thread(std::bind(&ServerDaemon::execute, this, std::placeholders::_1), NULL),
                               shouldStop(false)
{
    socketServer = SocketServer();
    socketClient = SocketClient("localhost", socketServer.getPort());

    userManagers = std::unordered_map<std::string, UserManager *>();
    clientManagers = std::unordered_map<std::string, std::unordered_map<unsigned long long, ClientManager *>>();
    Logger::log("ServerDaemon created");
}

void *ServerDaemon::execute(void *dummy)
{
    Logger::log("ServerDaemon started");
    socket = SocketServerSession(socketServer.listenAndAccept()); // Aceita conexão com o ConnectionManager -> utiliza a socketClient
    do
    {
        char *buffer = socket.read();

        switch (buffer[0])
        {
        case CLIENT_CONNECTED_MSG: // Mensagem de novo cliente -> recebido do ConnectionManager
            processClientConnectedMsg(buffer);
            break;
        case END_CLIENT_MSG: // Mensagem de desconexão do cliente
            processClientDisconnectedMsg(buffer);
            break;
        case STOP_MSG: // Para a thread
            shouldStop = true;
            break;
        default:
            Logger::log("Unknown message received: " + std::to_string(+buffer[0]));
            break;
        }
    } while (!shouldStop || (userManagers.size() > 0));

    for (auto &clientManagers : clientManagers)
    {
        for (auto &clientManager : clientManagers.second)
        {
            clientManager.second->stop();
            delete clientManager.second;
        }
    }

    for (auto &user : userManagers)
    {
        user.second->stopGraciously();
        user.second->join();
        delete user.second;
    }

    Logger::log("ServerDaemon stopped");
    return NULL;
}

void ServerDaemon::processClientConnectedMsg(const char *buffer)
{
    ClientConnectedMsg msg;
    msg.decode(buffer);
    std::string username = std::string(msg.username);
    Logger::log("Client connected: " + username + " " + std::to_string(clientManagers[username].size() + 1) + " " + msg.clientSocket->getClientIP() + ":" + std::to_string(msg.clientSocket->getClientPort()));

    if (clientManagers[username].size() >= MAX_CLIENTS_PER_USER)
    {
        Logger::log("User already has the maximum number of clients connected: " + username);
        RejectConnectMsg rejectMsg("User already has the maximum number of clients connected");
        char *buffer = rejectMsg.encode();
        msg.clientSocket->write(buffer);
        delete[] buffer;
    }
    else if (shouldStop)
    {
        RejectConnectMsg rejectMsg("Server is shutting down");
        char *buffer = rejectMsg.encode();
        socketClient.write(buffer);
        delete[] buffer;
    }
    else
    {
        if (userManagers.find(username) == userManagers.end())
        {
            userManagers[username] = new UserManager(username);
            userManagers[username]->start();
            SocketClient *userManagerSocket = userManagers[username]->getSocketClient();
            userManagerSocket->connect();
        }

        ClientManager *clientManager = new ClientManager(username, clientManagers[username].size() + 1, msg.clientSocket, userManagers[username]->getSocketClient(), &socketClient);
        clientManagers[username][clientManagers[username].size() + 1] = clientManager;
        clientManager->start();
    }
}

void ServerDaemon::processClientDisconnectedMsg(const char *buffer)
{
    EndClientMsg msg;
    msg.decode(buffer);
    std::string username = std::string(msg.username);
    unsigned long long clientId = msg.clientId;
    Logger::log("ServerDaemon: Client disconnected: " + username + " " + std::to_string(clientId));

    if (clientManagers.find(username) == clientManagers.end() || clientManagers[username].find(clientId) == clientManagers[username].end())
    {
        Logger::log("Client does not exist: " + username + " " + std::to_string(clientId));
    }
    else
    {
        ClientManager *toDelete = clientManagers[username][clientId];
        toDelete->join();
        clientManagers[username].erase(clientId);
        delete toDelete;
    }

    if (clientManagers[username].size() == 0)
    {
        UserManager *toDelete = userManagers[username];
        toDelete->stopGraciously();
        toDelete->join();
        userManagers.erase(username);
        delete toDelete;
    }
}

void ServerDaemon::stopGraciously()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    buffer[0] = STOP_MSG;
    socketClient.write(buffer);
    delete[] buffer;
    join();
}