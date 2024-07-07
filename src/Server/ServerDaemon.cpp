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
}

void *ServerDaemon::execute(void *dummy)
{
    socket = SocketServerSession(socketServer.listenAndAccept()); // Aceita conexão com o ConnectionManager -> utiliza a socketClient

    do
    {
        char *buffer = socket.read();

        switch (buffer[0])
        {
        case CLIENT_CONNECTED_MSG: // Mensagem de novo cliente
            processClientConnectedMsg(buffer);
            break;
        case END_CLIENT_MSG: // Mensagem de desconexão do cliente
            processClientDisconnectedMsg(buffer);
            break;
        case STOP_MSG: // Para a thread
            shouldStop = true;
            break;
        default:
            std::cerr << "Unknown message received: " << +buffer[0] << std::endl;
            break;
        }
    } while (!shouldStop && (userManagers.size() > 0));

    return NULL;
}

void ServerDaemon::processClientConnectedMsg(const char *buffer)
{
    ClientConnectedMsg msg;
    msg.decode(buffer);
    std::string username = std::string(msg.username);

    if (clientManagers[username].size() >= MAX_CLIENTS_PER_USER)
    {
        RejectConnectMsg rejectMsg("User already has the maximum number of clients connected");
        char *buffer = rejectMsg.encode();
        socketClient.write(buffer);
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

        ClientManager *clientManager = new ClientManager(username, clientManagers.size()+1, msg.clientSocket, userManagers[username]->getSocketClient(), &socketClient);
        clientManagers[username][clientManagers.size()+1] = clientManager;
        clientManager->start();
    }
}

void ServerDaemon::processClientDisconnectedMsg(const char *buffer)
{
    EndClientMsg msg;
    msg.decode(buffer);
    std::string username = std::string(msg.username);
    unsigned long long clientId = msg.clientId;

    if (clientManagers.find(username) == clientManagers.end() || clientManagers[username].find(clientId) == clientManagers[username].end())
    {
        std::cerr << "Client does not exist" << std::endl;
    }
    else
    {
        clientManagers[username][clientId]->join();
        delete clientManagers[username][clientId];
        clientManagers[username].erase(clientId);
    }

    if (clientManagers[username].size() == 0)
    {
        userManagers[username]->stopGraciously();
        userManagers[username]->join();
        delete userManagers[username];
        userManagers.erase(username);
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