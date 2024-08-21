#include "Server/BackupClient.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Protocol.hpp"

#include <string>

BackupClient::BackupClient(int id, bool backup, bool prevInRing, SocketClient *serverDaemon) : Thread(std::bind(&BackupClient::execute, this, std::placeholders::_1), NULL)
{
    this->id = id;
    this->serverDaemon = serverDaemon;
    this->backup = backup;
    this->prevInRing = prevInRing;
    this->participant = false;

    if (!backup || prevInRing)
        socketServer = SocketServer(/*NEXT_IN_RING_PORT*/);
    else
        socketServer = SocketServer();

    port = socketServer.getPort();
}

void *BackupClient::execute(void *dummy)
{
    Logger::log("BackupClient started");
    if (backup)
    {
        socket = SocketServerSession(socketServer.listenAndAccept());
        if (prevInRing)
        {
            while (true)
            {
                char *buffer = socket.read();
                // TODO: listen to election and elected messages
                delete[] buffer;
            }
        }
        else
        {
            while (true)
            {
                char *buffer = socket.read();
                Logger::log("BackupClient received message " + std::to_string(+(unsigned char)buffer[0]));
                // Redirects message to ServerDaemon
                serverDaemon->write(buffer);
                delete[] buffer;
            }
        }
    }
    else
    {
        ;// TODO: connect to backup server
    }
    // while (true)
    // {
    //     socket = SocketServerSession(socketServer.listenAndAccept());
    //     char *buffer = socket.read();
    //     serverDaemon->write(buffer);
    //     delete[] buffer;
    // }
    return NULL;
}

void BackupClient::connect(std::string address, int port)
{
    socketClient = SocketClient(address.c_str(), port);
    socketClient.connect();
}

void BackupClient::sendImAlive()
{
    char buffer[SOCKET_BUFFER_SIZE] = {PRIMARY_ALIVE_MSG};
    socketClient.write(buffer);
}

void BackupClient::sendElection(int id)
{
    int sendId = id == 0 ? this->id : id;

    if (!participant || id > this->id)
    {
        participant = true;
        ElectionMsg msg(sendId);
        char *buffer = msg.encode();
        socketClient.write(buffer);
        delete[] buffer;
    }
}

void BackupClient::sendElected(std::string address, int port, int id)
{
    int sendId = id == 0 ? this->id : id;

    ElectedMsg msg(sendId);
    char *buffer = msg.encode();
    socketClient.write(buffer);
    delete[] buffer;
}

void BackupClient::stop()
{
    socketServer.close();
    socketClient.close();
    socket.close();
    Thread::stop();
}