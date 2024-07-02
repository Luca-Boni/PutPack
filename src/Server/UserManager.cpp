#include "Server/UserManager.hpp"
#include "Utils/Protocol.hpp"
#include <unordered_map>
#include <iostream>

UserManager::UserManager(const std::string username, const int serverDaemonPort) : Thread(std::bind(&UserManager::execute, this, std::placeholders::_1), NULL),
                                                                                   username(username),
                                                                                   serverDaemonPort(serverDaemonPort),
                                                                                   shouldStop(false)
{
    socketServer = SocketServer();
    socketClient = SocketClient("localhost", socketServer.getPort());

    fileMutexes = MutexHash<std::string>();
    fileReaders = std::unordered_map<unsigned long long, FileReader *>();
    fileWriters = std::unordered_map<unsigned long long, FileWriter *>();
    fileWriterSessions = std::unordered_map<unsigned long long, SocketServerSession *>();
}

void *UserManager::execute(void *dummy)
{
    socket = SocketServerSession(socketServer.listenAndAccept());

    do
    {
        char *buffer = socket.read();
        
        if (buffer[0] = FILE_WRITE_MSG)
            processFileWriteMsg(buffer);
        else if (buffer[0] = FILE_READ_MSG)
            processFileReadMsg(buffer);
        else if (buffer[0] = STOP_MSG)
            shouldStop = true;
        else
            std::cerr << "Unknown message received: " << +buffer[0] << std::endl;
        
        delete[] buffer;
    }while (!shouldStop && (fileReaders.size() + fileWriters.size() > 0));

    return NULL;
}

void UserManager::processFileWriteMsg(const char *buffer)
{
    struct FileHandlerMessage msg;
    msg.decode(buffer);
    FileWriter *writer;
    if (fileWriters.find(msg.fileHandlerId) == fileWriters.end())
    {
        writer = new FileWriter(username, msg.clientId, fileMutexes.getOrAddMutex(msg.filename), msg.filename, socketServer.getPort());
        fileWriters[msg.fileHandlerId] = writer;
        writer->start();
        SocketServerSession* newSession = new SocketServerSession(socketServer.listenAndAccept());
        fileWriterSessions[msg.fileHandlerId] = newSession;
        newSession->write(buffer);
    }
    else
    {
        SocketServerSession *session = fileWriterSessions[msg.fileHandlerId];
        session->write(buffer);
        if (msg.size == 0)
        {
            writer = fileWriters[msg.fileHandlerId];
            writer->join();
            fileWriters.erase(msg.fileHandlerId);
            delete writer;

            session->close();
            fileWriterSessions.erase(msg.fileHandlerId);
            delete session;
        }
    }
    
}

void UserManager::processFileReadMsg(const char *buffer)
{
    return;
}

void UserManager::stop()
{
    // Awaits for all file modifications to finish
    for (auto &pair : fileWriters)
    {
        pair.second->join();
    }

    // Closes all file readers
    for (auto &pair : fileReaders)
    {
        pair.second->stop();
    }

    // Closes the sockets
    socket.close();
    socketServer.close();
    socketClient.close();

    // Stops the thread
    Thread::stop();
}

void UserManager::stopGraciously()
{
    char* buffer = new char[SOCKET_BUFFER_SIZE]();
    buffer[0] = STOP_MSG;
    socketClient.write(buffer);
    delete[] buffer;
}