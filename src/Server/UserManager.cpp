#include "Server/UserManager.hpp"
#include "Utils/Protocol.hpp"
#include "Server/ClientUserProtocol.hpp"
#include "Utils/FileDeleter.hpp"
#include <unordered_map>
#include <iostream>
#include <filesystem>

UserManager::UserManager(const std::string username) : Thread(std::bind(&UserManager::execute, this, std::placeholders::_1), NULL),
                                                       username(username),
                                                       shouldStop(false)
{
    files = std::vector<std::string>();

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
    for (const auto &file : std::filesystem::directory_iterator("./data/" + username))
        files.push_back(file.path().filename());

    do
    {
        char *buffer = socket.read();

        switch (buffer[0])
        {
        case NEW_CLIENT_MSG: // Mensagem de novo cliente
            processNewClientMsg(buffer);
            break;
        case END_CLIENT_MSG: // Mensagem de desconexão do cliente
            processEndClientMsg(buffer);
            break;
        case FILE_WRITE_MSG: // Escreve um arquivo no servidor -> mensagem vinda do ClientManager -> envia diretamnte para os clientes
            processFileWriteMsg(buffer);
            break;
        case FILE_READ_MSG: // Leu um arquivo do servidor e manda diretamente para o cliente, sem ClientManager
            processFileReadMsg(buffer);
            break;
        case SYNC_MSG: // Mensagem de sync_dir
            processSyncAllMsg(buffer);
            break;
        case FILE_DELETE_MSG: // Deleta um arquivo do servidor
            processFileDeleteMsg(buffer);
        case STOP_MSG: // Para a thread
            shouldStop = true;
            break;
        default:
            std::cerr << "Unknown message received: " << +buffer[0] << std::endl;
            break;
        }

        delete[] buffer;
    } while (!shouldStop && (fileReaders.size() + fileWriters.size() > 0));

    return NULL;
}

void UserManager::readAllFiles(unsigned long long clientId)
{
    for (std::string file : files)
    {
        FileReader *reader = new FileReader(username, clientId, fileMutexes.getOrAddMutex(file), file, &socketClient);
        reader->start();
        reader->waitTillConnect();
    }
}

void UserManager::processNewClientMsg(const char *buffer)
{
    NewClientMsg msg;
    msg.decode(buffer);
    if (clientManagerSockets.find(msg.clientId) != clientManagerSockets.end())
    {
        std::cerr << "Client already exists" << std::endl;
    }
    else
    {
        clientManagerSockets[msg.clientId] = msg.clientSocket;
        readAllFiles(msg.clientId);
    }
}

void UserManager::processEndClientMsg(const char *buffer)
{
    EndClientMsg msg;
    msg.decode(buffer);
    if (clientManagerSockets.find(msg.clientId) == clientManagerSockets.end())
    {
        std::cerr << "Client does not exist" << std::endl;
    }
    else
    {
        clientManagerSockets.erase(msg.clientId);
    }

    if (clientManagerSockets.size() == 0) // Para o UserManager se não houver mais clientes conectados
    {
        shouldStop = true;
    }
}

void UserManager::processSyncAllMsg(const char *buffer)
{
    SyncAllMsg msg;
    msg.decode(buffer);
    readAllFiles(msg.clientId);
}

void UserManager::processFileWriteMsg(const char *buffer)
{
    struct FileHandlerMessage msg;
    msg.decode(buffer);

    for (auto &client : clientManagerSockets) // Encaminha modificação para todos os clientes - exceto o que enviou a mensagem
    {
        if (client.first != msg.clientId)
        {
            client.second->write(buffer);
        }
    }

    FileWriter *writer;

    if (std::find(files.begin(), files.end(), msg.filename) == files.end()) // Caso o arquivo ainda não exista
    {
        files.push_back(msg.filename);
    }
    if (fileWriters.find(msg.fileHandlerId) == fileWriters.end()) // Caso o arquivo ainda não esteja sendo escrito
    {
        writer = new FileWriter(username, msg.clientId, fileMutexes.getOrAddMutex(msg.filename), msg.filename, socketServer.getPort());
        fileWriters[msg.fileHandlerId] = writer;
        writer->start();
        SocketServerSession *newSession = new SocketServerSession(socketServer.listenAndAccept());
        fileWriterSessions[msg.fileHandlerId] = newSession;
        newSession->write(buffer);
    }
    else // Caso o arquivo já esteja sendo escrito
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
    struct FileHandlerMessage msg;
    msg.decode(buffer);
    SocketServerSession *clientManagerSocket = clientManagerSockets[msg.clientId];
    clientManagerSocket->write(buffer);
}

void UserManager::processFileDeleteMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    FileDeleter deleter(username, msg.clientId, fileMutexes.getOrAddMutex(msg.filename), msg.filename);
    deleter.start();
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
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    buffer[0] = STOP_MSG;
    socketClient.write(buffer);
    delete[] buffer;
}