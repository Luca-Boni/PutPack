#pragma once

#include "Utils/Thread.hpp"
#include "Utils/SocketClient.hpp"
#include "Utils/SocketServer.hpp"
#include "Utils/SocketServerSession.hpp"
#include "Utils/Logger.hpp"

#include <ctime>

// #define NEXT_IN_RING_PORT 40000

class BackupClient : public Thread
// If it is a backup and a "next in ring", it doesn't run as thread, only serves to connect to the next server
{
private:
    int id;
    int port;
    bool backup;       // If it belongs to a backup or primary server
    bool prevInRing;   // If it is the previous server in the ring
    bool participant;
    clock_t lastAlive; // Last time the backup sent an alive message

    SocketClient *serverDaemon; // Socket to write to the server daemon

    SocketServer socketServer;  // Socket that the backup connects to
    SocketServerSession socket; // Socket to read from the backup

    SocketClient socketClient; // Socket to write to the backup

    void *execute(void *dummy);

public:
    BackupClient(int id, bool backup, bool prevInRing, SocketClient *serverDaemon);
    BackupClient() {}
    ~BackupClient() {}
    int getClientPort() { return port; }
    int getServerPort() { return socketServer.getPort(); }
    void connect(std::string address, int port); // Connects to another BackupClient
    void sendImAlive();
    void sendElection(int id=0);
    void sendElected(std::string address, int port, int id=0);

    void stop();
};