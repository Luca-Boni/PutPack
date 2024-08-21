#include "Server/UserManager.hpp"
#include "Utils/Protocol.hpp"
#include "Server/ClientUserProtocol.hpp"
#include "Utils/FileDeleter.hpp"
#include <unordered_map>
#include <iostream>
#include <filesystem>
#include <sys/stat.h>

UserManager::UserManager(const std::string username, bool backup) : Thread(std::bind(&UserManager::execute, this, std::placeholders::_1), NULL),
                                                                    username(username),
                                                                    shouldStop(false),
                                                                    backup(backup)
{
    files = std::vector<std::string>();

    socketServer = SocketServer();
    socketClient = SocketClient("localhost", socketServer.getPort());

    fileMutexes = MutexHash<std::string>();
    fileReaders = std::unordered_map<unsigned long long, std::unordered_map<unsigned long long, FileReader *>>();
    fileWriters = std::unordered_map<unsigned long long, std::unordered_map<unsigned long long, FileWriter *>>();
    fileWriterSessions = std::unordered_map<unsigned long long, SocketServerSession *>();
    isSyncing = std::unordered_map<unsigned long long, bool>();
    Logger::log("UserManager created: " + username);
}

void *UserManager::execute(void *dummy)
{
    Logger::log("UserManager started: " + username);
    socket = SocketServerSession(socketServer.listenAndAccept());

    char path[FILENAME_MAX];
    ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
    std::string exe_dir = std::filesystem::path(std::string(path, (count > 0) ? count : 0)).parent_path().string();

    if (!std::filesystem::exists(exe_dir + "/sync_dir_" + username + "/"))
    {
        std::filesystem::create_directory(exe_dir + "/sync_dir_" + username + "/");
    }

    for (const auto &file : std::filesystem::directory_iterator(exe_dir + "/sync_dir_" + username + "/"))
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
            break;
        case FILE_DOWNLOAD_MSG: // Envia um arquivo para o cliente por demanda
            processFileDownloadMsg(buffer);
            break;
        case LIST_SERVER_FILES_MSG:
            processListServerFilesMsg(buffer);
            break;
        case SET_TO_PRIMARY_MSG:
            processSetToPrimaryMsg(buffer);
            break;
        case STOP_MSG: // Para a thread
            shouldStop = true;
            break;
        default:
            Logger::log("Unknown message received: " + username + std::to_string(+buffer[0]));
            break;
        }

        delete[] buffer;
    } while (!shouldStop || (fileReaders.size() + fileWriters.size() > 0));

    Logger::log("UserManager stopped: " + username);
    return NULL;
}

void UserManager::readAllFiles(unsigned long long clientId)
{
    isSyncing[clientId] = true;
    for (std::string file : files)
    {
        FileReader *reader = new FileReader(username, clientId, fileMutexes.getOrAddMutex(file), file, &socketClient);
        reader->start();
        reader->waitTillConnect();
        if (fileReaders.find(clientId) == fileReaders.end())
        {
            fileReaders[clientId] = std::unordered_map<unsigned long long, FileReader *>();
        }
        fileReaders[clientId][reader->getId()] = reader;
    }
}

void UserManager::processNewClientMsg(const char *buffer)
{
    NewClientMsg msg;
    msg.decode(buffer);
    Logger::log("New client connected: " + username + std::to_string(msg.clientId));
    if (clientManagerSockets.find(msg.clientId) != clientManagerSockets.end())
    {
        Logger::log("Client already exists: " + username + std::to_string(msg.clientId));
    }
    else if (!backup)
    {
        clientManagerSockets[msg.clientId] = msg.clientSocket;
        char *newBuffer = new char[SOCKET_BUFFER_SIZE]();
        newBuffer[0] = CLIENT_CONNECTED_MSG;
        msg.clientSocket->write(buffer);
        readAllFiles(msg.clientId);
    }
}

void UserManager::processEndClientMsg(const char *buffer)
{
    EndClientMsg msg;
    msg.decode(buffer);
    Logger::log("UserManager: Client disconnected: " + username + " " + std::to_string(msg.clientId));
    if (clientManagerSockets.find(msg.clientId) == clientManagerSockets.end())
    {
        Logger::log("Client does not exist: " + username + std::to_string(msg.clientId));
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
    Logger::log("Syncing all files: " + username + std::to_string(msg.clientId));
    readAllFiles(msg.clientId);
}

void UserManager::processFileWriteMsg(const char *buffer)
{
    struct FileHandlerMessage msg;
    msg.decode(buffer);

    if (!backup)
    {
        char *okBuffer = new char[SOCKET_BUFFER_SIZE]();
        okBuffer[0] = OK_MSG;
        clientManagerSockets[msg.clientId]->write(okBuffer);
        delete[] okBuffer;
    }

    Logger::log("Writing to file: " + username + " " + std::to_string(msg.clientId) + " " + msg.filename + " " + std::to_string(msg.size) + " bytes");
    if (!backup)
    {
        for (auto &client : clientManagerSockets) // Encaminha modificação para todos os clientes - exceto o que enviou a mensagem
        {
            if (client.first != msg.clientId)
            {
                client.second->write(buffer);
            }
        }
    }

    FileWriter *writer;

    if (std::find(files.begin(), files.end(), msg.filename) == files.end()) // Caso o arquivo ainda não exista
    {
        files.push_back(msg.filename);
    }

    if (fileWriters.find(msg.clientId) == fileWriters.end())
    {
        fileWriters[msg.clientId] = std::unordered_map<unsigned long long, FileWriter *>();
    }

    if (fileWriters[msg.clientId].find(msg.fileHandlerId) == fileWriters[msg.clientId].end()) // Caso o arquivo ainda não esteja sendo escrito
    {
        writer = new FileWriter(username, msg.clientId, fileMutexes.getOrAddMutex(msg.filename), msg.filename, socketServer.getPort());
        fileWriters[msg.clientId][writer->getId()] = writer;
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
            writer = fileWriters[msg.clientId][msg.fileHandlerId];
            writer->join();
            fileWriters[msg.clientId].erase(msg.clientId);

            if (fileWriters[msg.clientId].size() == 0)
            {
                fileWriters.erase(msg.clientId);
            }

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

    if (msg.size == 0)
    {
        fileReaders[msg.clientId].erase(msg.fileHandlerId);
        if (fileReaders[msg.clientId].size() == 0)
        {
            fileReaders.erase(msg.clientId);
        }
    }

    // Logger::log("Reading file: " + username + " " + std::to_string(msg.clientId) + " " + msg.filename);
    SocketServerSession *clientManagerSocket = clientManagerSockets[msg.clientId];
    if (!backup)
        clientManagerSocket->write(buffer);

    if (msg.size == 0 && fileReaders.find(msg.clientId) == fileReaders.end())
    {
        if (isSyncing[msg.clientId])
        {
            isSyncing[msg.clientId] = false;
            // Finished syncing all files
            if (!backup)
            {
                char *buffer = new char[SOCKET_BUFFER_SIZE]();
                buffer[0] = SYNC_FINISHED_MSG;
                clientManagerSockets[msg.clientId]->write(buffer);
                delete[] buffer;
            }

            Logger::log("Finished syncing client " + std::to_string(msg.clientId));
        }
    }
}

void UserManager::processFileDeleteMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    Logger::log("Deleting file: " + username + " " + std::to_string(msg.clientId) + " " + msg.filename);

    if (!backup)
    {
        for (auto &client : clientManagerSockets) // Encaminha modificação para todos os clientes - exceto o que enviou a mensagem
        {
            if (client.first != msg.clientId)
            {
                client.second->write(buffer);
            }
        }
    }

    FileDeleter deleter(username, msg.clientId, fileMutexes.getOrAddMutex(msg.filename), msg.filename);
    deleter.start();
    deleter.join();
}

void UserManager::processFileDownloadMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    std::string filename = filenameFromPath(msg.filename);
    Logger::log("Sending file as download: " + username + " " + std::to_string(msg.clientId) + " " + filename);
    FileReader *reader = new FileReader(username, msg.clientId, fileMutexes.getOrAddMutex(filename), msg.filename, &socketClient);
    reader->start();
}

void UserManager::stop()
{
    // Awaits for all file modifications to finish
    for (auto &pair : fileWriters)
    {
        for (auto &fileWriter : pair.second)
        {
            fileWriter.second->stop();
        }
    }

    // Closes all file readers
    for (auto &pair : fileReaders)
    {
        for (auto &fileReader : pair.second)
        {
            fileReader.second->stop();
        }
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

std::string getFileAndMACTimes(const std::filesystem::path &filepath)
{
    struct stat fileStat;
    stat(filepath.c_str(), &fileStat);
    struct tm *mtimeinfo = localtime(&fileStat.st_mtime);
    struct tm *atimeinfo = localtime(&fileStat.st_atime);
    struct tm *ctimeinfo = localtime(&fileStat.st_ctime);

    char mtime[80];
    char atime[80];
    char ctime[80];

    strftime(mtime, 80, "%d-%m-%Y %H:%M:%S", mtimeinfo);
    strftime(atime, 80, "%d-%m-%Y %H:%M:%S", atimeinfo);
    strftime(ctime, 80, "%d-%m-%Y %H:%M:%S", ctimeinfo);

    std::string out = "";

    out = "Filename: " + filepath.filename().string() + "\n" + "    Last modified: " + mtime + "\n" + "    Last accessed: " + atime + "\n" + "          Created: " + ctime + "\n";

    return out;
}

void UserManager::processListServerFilesMsg(const char *buffer)
{
    ListServerCommandMsg msg;
    msg.decode(buffer);
    Logger::log("Listing server files: " + username + " " + std::to_string(msg.clientId));
    std::string filesInfo = "";

    char path[FILENAME_MAX];
    ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
    std::string exe_dir = std::filesystem::path(std::string(path, (count > 0) ? count : 0)).parent_path().string();

    for (const auto &file : std::filesystem::directory_iterator(exe_dir + "/sync_dir_" + username + "/"))
    {
        filesInfo += getFileAndMACTimes(file.path());
    }

    msg.data = filesInfo;
    if (!backup)
    {
        char *newBuffer = msg.encode();
        newBuffer[0] = LIST_SERVER_FILES_MSG;
        clientManagerSockets[msg.clientId]->write(newBuffer);
        delete[] newBuffer;
    }
}

void UserManager::setToPrimary()
{
    char *buffer = new char[SOCKET_BUFFER_SIZE]();
    buffer[0] = SET_TO_PRIMARY_MSG;
    socketClient.write(buffer);
    delete[] buffer;
}

void UserManager::processSetToPrimaryMsg(const char *buffer)
{
    Logger::log("UserManager: Setting to primary " + username);
    backup = false;
}

void UserManager::setClientManagerSockets(std::unordered_map<unsigned long long, SocketServerSession *> clientManagerSockets)
{
    this->clientManagerSockets = clientManagerSockets;
}