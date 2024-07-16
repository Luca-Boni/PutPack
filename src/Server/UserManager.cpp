#include "Server/UserManager.hpp"
#include "Utils/Protocol.hpp"
#include "Server/ClientUserProtocol.hpp"
#include "Utils/FileDeleter.hpp"
#include <unordered_map>
#include <iostream>
#include <filesystem>
#include <sys/stat.h>

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
        case STOP_MSG: // Para a thread
            shouldStop = true;
            break;
        default:
            std::cerr << "Unknown message received: " << +buffer[0] << std::endl;
            break;
        }

        delete[] buffer;
    } while (!shouldStop || (fileReaders.size() + fileWriters.size() > 0));

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
    std::cout << "Syncing all files: User: " << username << "; Client: " << msg.clientId << std::endl;
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
    if (fileWriters.find(msg.clientId) == fileWriters.end()) // Caso o arquivo ainda não esteja sendo escrito
    {
        writer = new FileWriter(username, msg.clientId, fileMutexes.getOrAddMutex(msg.filename), msg.filename, socketServer.getPort());
        fileWriters[msg.clientId] = writer;
        writer->start();
        SocketServerSession *newSession = new SocketServerSession(socketServer.listenAndAccept());
        fileWriterSessions[msg.clientId] = newSession;
        newSession->write(buffer);
    }
    else // Caso o arquivo já esteja sendo escrito
    {
        SocketServerSession *session = fileWriterSessions[msg.clientId];
        session->write(buffer);
        if (msg.size == 0)
        {
            writer = fileWriters[msg.clientId];
            writer->join();
            fileWriters.erase(msg.clientId);
            delete writer;

            session->close();
            fileWriterSessions.erase(msg.clientId);
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

    for (auto &client : clientManagerSockets) // Encaminha modificação para todos os clientes - exceto o que enviou a mensagem
    {
        if (client.first != msg.clientId)
        {
            client.second->write(buffer);
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
    FileReader *reader = new FileReader(username, msg.clientId, fileMutexes.getOrAddMutex(filename), msg.filename, &socketClient);
    reader->start();
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

    out + "Filename: " + filepath.filename().string() + "\n";
    out + "    Last modified: " + mtime + "\n";
    out + "    Last accessed: " + atime + "\n";
    out + "          Created: " + ctime + "\n";

    return out;
}

void UserManager::processListServerFilesMsg(const char* buffer)
{
    ListServerCommandMsg msg;
    msg.decode(buffer);
    std::string filesInfo = "";

    char path[FILENAME_MAX];
    ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
    std::string exe_dir = std::filesystem::path(std::string(path, (count > 0) ? count : 0)).parent_path().string();

    for (const auto &file : std::filesystem::directory_iterator(exe_dir + "/sync_dir_" + username + "/"))
    {
        filesInfo += getFileAndMACTimes(file.path());
    }

    msg.data = filesInfo;
    char *newbuffer = msg.encode();
    clientManagerSockets[msg.clientId]->write(newbuffer);
    delete[] newbuffer;
}