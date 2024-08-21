#include "Server/ServerDaemon.hpp"
#include "Utils/Protocol.hpp"
#include "Server/ClientUserProtocol.hpp"

#include <ctime>
#include <iostream>

#define MAX_CLIENTS_PER_USER 2

ServerDaemon::ServerDaemon(int id, bool backup, std::string primaryAddress) : Thread(std::bind(&ServerDaemon::execute, this, std::placeholders::_1), NULL),
                                                                              shouldStop(false),
                                                                              id(id),
                                                                              backup(backup)
{
    this->ownPort = 0;
    if (backup)
    {
        this->primaryAddress = primaryAddress.substr(0, primaryAddress.find(":"));
        this->primaryPort = std::stoi(primaryAddress.substr(primaryAddress.find(":") + 1));
    }

    socketServer = SocketServer();
    socketClient = SocketClient("localhost", socketServer.getPort());

    userManagers = std::unordered_map<std::string, UserManager *>();
    clientManagers = std::unordered_map<std::string, std::unordered_map<unsigned long long, ClientManager *>>();

    backupServers = std::vector<BackupInfo>();

    // aliveTimer = AliveTimer(10, std::bind(&ServerDaemon::aliveTimerCallback, this, std::placeholders::_1));

    Logger::log("ServerDaemon created");
}

void *ServerDaemon::execute(void *dummy)
{
    Logger::log("ServerDaemon started");
    // aliveTimer.start();
    socket = SocketServerSession(socketServer.listenAndAccept()); // Aceita conexão com o ConnectionManager -> utiliza a socketClient

    while (ownPort == 0)
        ;

    if (backup)
    {
        connectToPrimary();
    }

    do
    {
        char *buffer = socket.read();

        if ((unsigned char)buffer[0] < SEND_CLI_REDIRECT_MSG)
        {
            switch (buffer[0])
            {
            case CLIENT_CONNECTED_MSG: // Mensagem de novo cliente -> recebido do ConnectionManager
                processClientConnectedMsg(buffer);
                break;
            case END_CLIENT_MSG: // Mensagem de desconexão do cliente
                processClientDisconnectedMsg(buffer);
                break;
            case NEW_SERVER_MSG:
                processNewServerMsg(buffer);
                break;
            case NEXT_IN_RING_MSG:
                processNextInRingMsg(buffer);
                break;
            case SET_TO_PRIMARY_MSG:
                processSetToPrimaryMsg(buffer);
                break;
            case SET_OWN_ADDRESS_MSG:
            {
                SetOwnAddressMsg msg;
                msg.decode(buffer);
                ownAddress = msg.address;
                Logger::log("Own address set: " + ownAddress);
            }
                break;
            case STOP_MSG: // Para a thread
                shouldStop = true;
                break;
            default:
                Logger::log("Unknown message received: " + std::to_string(+buffer[0]));
                break;
            }
        }
        else if ((unsigned char)buffer[0] < RECV_CLI_REDIRECT_MSG)
        {
            buffer[0] -= SEND_CLI_REDIRECT_MSG;
            buffer[0] += RECV_CLI_REDIRECT_MSG;
            // Redirects messages to backup servers
            for (auto &backupServer : backupServers)
            {
                backupServer.socket->write(buffer);
            }
        }
        else if ((unsigned char)buffer[0] >= RECV_CLI_REDIRECT_MSG)
        {
            processMsgFromPrimary(buffer);
        }

        delete[] buffer;
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
    
    if (!backup)
    {
        // Redirects messages to backup servers
        strcpy(msg.clientAddress, socket.getClientIP().c_str());
        char *newBuffer = msg.encode();

        for (auto &backupServer : backupServers)
        {
            backupServer.socket->write(newBuffer);
        }
        delete[] newBuffer;
    }

    std::string username = std::string(msg.username);
    if (backup)
    {
        msg.clientSocket = NULL;
    }
    else
    {
        Logger::log("Client connected: " + username + " " + std::to_string(clientManagers[username].size() + 1) + " " + msg.clientAddress + ":" + std::to_string(msg.clientSocket->getClientPort()));
    }

    if (clientManagers[username].size() >= MAX_CLIENTS_PER_USER)
    {
        Logger::log("User already has the maximum number of clients connected: " + username);
        if (!backup)
        {
            RejectConnectMsg rejectMsg("User already has the maximum number of clients connected");
            char *buffer = rejectMsg.encode();
            msg.clientSocket->write(buffer);
            delete[] buffer;
        }
    }
    else if (shouldStop && !backup)
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
            userManagers[username] = new UserManager(username, backup);
            userManagers[username]->start();
            SocketClient *userManagerSocket = userManagers[username]->getSocketClient();
            userManagerSocket->connect();
        }

        ClientManager *clientManager = new ClientManager(username, clientManagers[username].size() + 1, msg.clientSocket, userManagers[username]->getSocketClient(), &socketClient, backup, std::string(msg.clientAddress), msg.FEPort, ownAddress);

        SocketClient *clientManagerSocket;
        if (backup)
        {
            clientManagerSocket = clientManager->getSocketClient();
        }
        
        clientManagers[username][clientManagers[username].size() + 1] = clientManager;
        clientManager->start();
        
        if (backup)
        {
            clientManagerSocket->connect();
        }
    }
}

void ServerDaemon::processClientDisconnectedMsg(const char *buffer)
{
    if (!backup)
    {
        // Redirects messages to backup servers
        for (auto &backupServer : backupServers)
        {
            backupServer.socket->write(buffer);
        }
    }

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

void *ServerDaemon::aliveTimerCallback(void *dummy)
{
    Logger::log("ServerDaemon: Alive timer callback");
    if (!backup)
    {
        char *buffer = new char[SOCKET_BUFFER_SIZE]();
        buffer[0] = PRIMARY_ALIVE_MSG;

        for (auto &backupInfo : backupServers)
        {
            backupInfo.socket->write(buffer);
        }

        delete[] buffer;
    }
    else
    {
        char *buffer = new char[SOCKET_BUFFER_SIZE]();
        buffer[0] = CHECK_PRIMARY_ALIVE_MSG;
        socketClient.write(buffer);
        delete[] buffer;
    }
    return NULL;
}

void ServerDaemon::connectToPrimary()
{
    /*
     * This works as a 5-step process:
     * 1. Create a socket client to connect to the primary server; the ConnectionManager listens and accepts the connection
     * 2. Meanwhile, a BackupClient is created and listens for connections - the NewServerMsg is sent to the primary containing the BackupClient's port
     * 3. The primary ConnectionManager sends the NewServerMsg to the primary ServerDaemon
     * 4. The primary ServerDaemon adds the new server to its list and creates a BackupClient that point to the backup's BackupClient
     * 5. The primary BackupClient connects to the backup BackupClient
     *
     * Meanwhile, the ServerDaemon gets the next server in the ring and sends back through the temporary socket
     */

    primaryServerSocket = SocketClient(primaryAddress.c_str(), primaryPort);
    primaryServerSocket.connect();
    Logger::log("Connected to primary");

    primaryServer = new BackupClient(id, true, false, &socketClient);
    primaryServer->start();

    NewServerMsg msg;
    msg.id = id;
    msg.port = primaryServer->getServerPort(); // Ip is written by ConnectionManager
    char *buffer = msg.encode();
    primaryServerSocket.write(buffer);
    delete[] buffer;
    Logger::log("Sent NewServerMsg to primary");
}

void ServerDaemon::processNextInRingMsg(const char *buffer)
{
    NextInRingMsg msg;
    msg.decode(buffer);
    Logger::log("Next in ring: " + std::to_string(msg.id) + "; " + msg.address + ":" + std::to_string(msg.port));

    ownAddress = msg.ownAddress;

    prevInRing = new BackupClient(id, false, true, &socketClient);
    prevInRing->start();

    nextInRing = new BackupClient(id, false, false, &socketClient);
    nextInRing->connect(msg.address, msg.port);
}

void ServerDaemon::processNewServerMsg(const char *buffer)
{
    NewServerMsg msg;
    msg.decode(buffer);
    Logger::log("New server connected: " + std::to_string(msg.id) + "; " + msg.address + ":" + std::to_string(msg.port));

    // É inserido o servidor na lista
    BackupInfo info(msg.id, msg.address, msg.port, msg.socket);

    SetOwnAddressMsg msg2(msg.address);
    char *buffer2 = msg2.encode();
    msg.socket->write(buffer2);
    delete[] buffer2;

    if (backupServers.size() > 0)
    {
        // Precisamos enviar para o backup que o próximo no anel é o primeiro da lista
        // Logger::log("Sending next in ring to new server");
        // BackupInfo nextInfo = backupServers[0];
        // NextInRingMsg newMsg(nextInfo.id, nextInfo.address, nextInfo.port, msg.address, msg.port);
        // char *newBuffer = newMsg.encode();
        // nextSocket.write(newBuffer);
        // delete[] newBuffer;

        // // Precisamos enviar para o anterior a mensagem de que esse novo backup é o próximo do anel
        // Logger::log("Sending new server to previous");
        // BackupInfo thisInfo = backupServers.back();
        // NextInRingMsg newMsg2(msg.id, msg.address, msg.port, thisInfo.address, thisInfo.port);
        // newBuffer = newMsg2.encode();
        // SocketClient prevSocket = SocketClient(thisInfo.address.c_str(), thisInfo.port);
        // prevSocket.connect();
        // prevSocket.write(newBuffer);
        // delete[] newBuffer;
    }
    backupServers.push_back(info);
}

void ServerDaemon::processMsgFromPrimary(const char *buffer)
{
    if (backup)
    {
        Logger::log("Redirecting message from primary: " + std::to_string(+(unsigned char)buffer[0]));
        unsigned char *newBuffer = new unsigned char[SOCKET_BUFFER_SIZE]();
        memcpy(newBuffer, buffer, SOCKET_BUFFER_SIZE);
        newBuffer[0] -= RECV_CLI_REDIRECT_MSG;
        switch (newBuffer[0])
        {
            case FILE_DELETE_MSG:
            case FILE_WRITE_MSG:
            case FILE_UPLOAD_MSG:
            case FILE_DEL_CMD_MSG:
            { // inside brackets to stop syntax highlighting error
                FileHandlerMessage msg;
                msg.decode((char*)newBuffer);
                if (msg.clientId != 0)
                {
                    ClientManager *clientManager = clientManagers[msg.username][msg.clientId];
                    if (clientManager != NULL)
                    {
                        SocketClient *clientSocket = clientManager->getSocketClient();
                        if (clientSocket != NULL)
                        {
                            clientSocket->write((char *)newBuffer);
                        }
                    }
                }
                else
                {
                    for (auto &clientManager : clientManagers[msg.username])
                    {
                        clientManager.second->getSocketClient()->write((char *)newBuffer);
                    }
                }
            }
                break;
            case END_CLIENT_MSG:
            {
                EndClientMsg msg;
                msg.decode((char*)newBuffer);
                clientManagers[msg.username][msg.clientId]->getSocketClient()->write(buffer);
            }
                break;
            default:
                break;
        }
        delete[] newBuffer;
    }
}

void ServerDaemon::processSetToPrimaryMsg(const char *buffer)
{
    backup = false;
    std::unordered_map<unsigned long long, SocketServerSession *> clientManagerSockets;

    for (auto &user : userManagers)
    {
        clientManagerSockets = std::unordered_map<unsigned long long, SocketServerSession *>();
        for (auto &clientManager : clientManagers[user.first])
        {
            clientManager.second->setToPrimary();
            clientManagerSockets[clientManager.first] = clientManager.second->getSocketServerSession();
        }
        user.second->setToPrimary();
        user.second->setClientManagerSockets(clientManagerSockets);
    }
}

void ServerDaemon::setToPrimary()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    buffer[0] = SET_TO_PRIMARY_MSG;
    socketClient.write(buffer);
    delete[] buffer;
}