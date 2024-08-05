#include "Client/ClientDaemon.hpp"
#include "Utils/Protocol.hpp"
#include "Client/FileMonitorProtocol.hpp"
#include "Utils/FileHandlerProtocol.hpp"
#include "Utils/FileDeleter.hpp"
#include "Client/ClientInterfaceProtocol.hpp"
#include "Utils/Logger.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <sys/stat.h>

ClientDaemon::ClientDaemon(const std::string &username, SocketClient *serverSocket) : Thread(std::bind(&ClientDaemon::execute, this, std::placeholders::_1), NULL),
                                                                                      username(username),
                                                                                      serverSocket(serverSocket),
                                                                                      isConnected(false),
                                                                                      shouldStop(false)
{
    socket = SocketServer();
    socketClient = SocketClient("localhost", socket.getPort());

    filesBeingRead = std::unordered_set<std::string>();
    filesBeingWritten = std::unordered_set<std::string>();

    fileMutexes = MutexHash<std::string>();

    fileReaders = std::unordered_map<std::string, FileReader *>();
    fileWriters = std::unordered_map<std::string, FileWriter *>();
    fileWriterSessions = std::unordered_map<std::string, SocketServerSession *>();

    char path[FILENAME_MAX];
    ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
    std::string exe_dir = std::filesystem::path(std::string(path, (count > 0) ? count : 0)).parent_path().string();

    syncDir = exe_dir + "/sync_dir_" + username + "/";

    fileMonitor = new FileMonitor(syncDir, &socketClient);
    Logger::log("ClientDaemon created: " + username);
}

void *ClientDaemon::execute(void *dummy)
{
    Logger::log("ClientDaemon started");
    if (!std::filesystem::exists(syncDir))
    {
        std::filesystem::create_directory(syncDir);
    }

    for (const auto &entry : std::filesystem::directory_iterator(syncDir))
    {
        FileDeleter deleter = FileDeleter(username, 0, fileMutexes.getOrAddMutex(entry.path().filename()), entry.path().filename());
        deleter.start();
        deleter.join();
    }

    socketSession = SocketServerSession(socket.listenAndAccept());
    ClientConnectedMsg msg(username.c_str());
    char *buffer = msg.encode();
    serverSocket->write(buffer);
    delete[] buffer;

    fileMonitor->start();

    while (!shouldStop || (filesBeingRead.size() > 0))
    {
        char *buffer = socketSession.read();

        switch ((unsigned char)buffer[0])
        {
        case REJECT_CONNECT_MSG: // Conexão rejeitada no servidor
            processConnectionRejectedMsg(buffer);
            break;
        case INTERFACE_COMMAND_MSG: // Comando vindo da interface
            processInterfaceCommand(buffer);
            break;
        case FILE_MONITOR_MSG: // Arquivo foi modificado/adicionado/deletado na sync_dir
            processFileMonitorMsg(buffer);
            break;
        case FILE_READ_MSG: // Arquivo lido -> envia mensagem para o servidor como escrita
            processFileReadMsg(buffer);
            break;
        // Arquivos modificados no servidor
        case FILE_WRITE_MSG: // Arquivo escrito no servidor -> deve escrever também no cliente
            processFileWriteMsg(buffer);
            break;
        case FILE_DELETE_MSG: // Arquivo deletado no servidor -> deve deletar também no cliente
            processFileDeleteMsg(buffer);
            break;
        // Arquivos modificados via comando
        case FILE_UPLOAD_MSG: // Arquivo enviado para o servidor -> deve sincronizar em todos os clientes
            processFileUploadMsg(buffer);
            break;
        case LIST_SERVER_FILES_MSG:
            processListServerFilesMsg(buffer);
        case SERVER_DEAD:
            Logger::log("Server is dead.");
            shouldStop = true;
            break;
        default:
            Logger::log("Unknown message received: " + std::to_string(+buffer[0]));
            break;
        }

        delete[] buffer;
    }

    fileMonitor->stop();

    serverSocket->close();
    Logger::log("ClientDaemon stopped.");

    return NULL;
}

void ClientDaemon::endClient()
{
    // Envia mensagem que o cliente está terminando
    EndClientMsg msg(0, username.c_str());
    char *buffer = msg.encode();
    serverSocket->write(buffer);
    delete[] buffer;

    shouldStop = true;
    Logger::log("Waiting for files to stop updating...");
    std::cout << "Waiting for files to stop updating..." << std::endl;
}

void ClientDaemon::processConnectionRejectedMsg(const char *buffer)
{
    RejectConnectMsg msg = RejectConnectMsg();
    msg.decode(buffer);
    Logger::log("Connection rejected: " + std::string(msg.message));
    std::cerr << "Connection rejected: " << msg.message << std::endl;
    exit(0);
}

void ClientDaemon::processFileMonitorMsg(const char *buffer)
{
    FileMonitorMsg msg;
    msg.decode(buffer);
    std::string filename = msg.filename;
    Logger::log("ClientDaemon received from FileMonitor: " + filename + " " + std::to_string(msg.event));

    if (msg.event == FileMonitorProtocol::FMP_FILE_CHANGE)
    {
        if (filesBeingRead.find(filename) == filesBeingRead.end())
        {
            filesBeingRead.insert(filename);

            FileReader *reader = new FileReader(username, 0, fileMutexes.getOrAddMutex(filename), filename, &socketClient);
            fileReaders[filename] = reader;
            reader->start();
        }
        else // Caso já estivesse lendo o arquivo, para e cria um novo reader para ler o arquivo do início
        {
            FileReader *reader = fileReaders[filename];
            reader->stop();

            FileHandlerMessage msg(0, filename.c_str(), 0, NULL); // Envia mensagem de fim de arquivo para o servidor
            char *buffer = msg.encode();
            socketClient.write(buffer);
            delete[] buffer;

            delete reader;
            reader = new FileReader(username, 0, fileMutexes.getOrAddMutex(filename), filename, &socketClient); // Começa a ler o arquivo novamente
            reader->start();
            fileReaders[filename] = reader;
        }
    }
    else if (msg.event == FileMonitorProtocol::FMP_FILE_REMOVE)
    {
        if (filesBeingRead.find(filename) != filesBeingRead.end()) // Caso estivesse lendo o arquivo, para a leitura
        {
            filesBeingRead.erase(filename);
            FileReader *reader = fileReaders[filename];
            reader->stop();
            fileReaders.erase(filename);

            FileHandlerMessage msg(0, filename.c_str(), 0, NULL); // Envia mensagem de fim de arquivo para o servidor
            char *buffer = msg.encode();
            socketClient.write(buffer);
            delete[] buffer;
            delete reader;
        }

        fileMonitor->disableFile(filename);
        FileDeleter deleter = FileDeleter(username, 0, fileMutexes.getOrAddMutex(filename), filename);
        deleter.start();
        deleter.join();
        fileMonitor->enableFile(filename);

        FileHandlerMessage msg(0, filename.c_str(), 0, NULL);
        char *buffer = msg.encode();
        buffer[0] = FILE_DELETE_MSG;
        serverSocket->write(buffer);
        delete[] buffer;
    }
}

void ClientDaemon::processFileReadMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    Logger::log("Sending to server file: " + std::string(msg.filename) + " " + std::to_string(msg.size) + " bytes");

    if (msg.size == 0)
    {
        filesBeingRead.erase(msg.filename);
    }

    char *newBuffer = msg.encode();
    newBuffer[0] = FILE_WRITE_MSG; // Envia mensagem de escrita para o servidor
    serverSocket->write(newBuffer);
    delete[] newBuffer;
}

void ClientDaemon::processFileWriteMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    Logger::log("Received file from server: " + std::string(msg.filename) + " " + std::to_string(msg.size) + " bytes");

    fileMonitor->disableFile(msg.filename);
    filesBeingWritten.insert(msg.filename);

    if (fileWriters.find(msg.filename) == fileWriters.end()) // Caso o arquivo ainda não esteja sendo escrito
    {
        FileWriter *writer = new FileWriter(username, msg.clientId, fileMutexes.getOrAddMutex(msg.filename), msg.filename, socket.getPort());
        fileWriters[msg.filename] = writer;
        writer->start();
        SocketServerSession *newSession = new SocketServerSession(socket.listenAndAccept());
        fileWriterSessions[msg.filename] = newSession;

        char *newBuffer = msg.encode();
        newSession->write(newBuffer);
        delete[] newBuffer;
    }
    else // Caso o arquivo já esteja sendo escrito
    {
        SocketServerSession *session = fileWriterSessions[msg.filename];
        char *newBuffer = msg.encode();
        session->write(newBuffer);
        delete[] newBuffer;

        if (msg.size == 0)
        {
            FileWriter *writer = fileWriters[msg.filename];
            writer->join();
            fileWriters.erase(msg.filename);
            delete writer;

            fileMonitor->enableFile(msg.filename);
            filesBeingWritten.erase(msg.filename);

            session->close();
            fileWriterSessions.erase(msg.filename);
            delete session;
        }
    }
}

void ClientDaemon::processFileDeleteMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    Logger::log("File deleted in the server: " + std::string(msg.filename));

    fileMonitor->disableFile(msg.filename);

    FileDeleter deleter = FileDeleter(username, msg.clientId, fileMutexes.getOrAddMutex(msg.filename), msg.filename);
    deleter.start();
    deleter.join();

    fileMonitor->enableFile(msg.filename);
}

void ClientDaemon::processFileUploadMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    Logger::log("File uploading to the server: " + std::string(msg.filename) + " " + std::to_string(msg.size) + " bytes");

    if (msg.size == 0)
    {
        filesBeingRead.erase(msg.filename);
    }
    else
    {
        filesBeingRead.insert(msg.filename);
    }

    serverSocket->write(buffer);
}

void ClientDaemon::processInterfaceCommand(const char *buffer)
{
    InterfaceCommandMsg msg;
    msg.decode(buffer);

    switch (msg.command)
    {
    case InterfaceCommand::UPLOAD:
    {
        UploadCommandMsg uploadMsg;
        uploadMsg.decode(buffer);
        uploadFile(uploadMsg.path);
        break;
    }
    case InterfaceCommand::DOWNLOAD:
    {
        DownloadCommandMsg downloadMsg;
        downloadMsg.decode(buffer);
        downloadFile(downloadMsg.filename);
        break;
    }
    case InterfaceCommand::DELETE:
    {
        DeleteCommandMsg deleteMsg;
        deleteMsg.decode(buffer);
        deleteFile(deleteMsg.filename);
        break;
    }
    case InterfaceCommand::LIST_SERVER:
    {
        listServerFiles();
        break;
    }
    case InterfaceCommand::LIST_CLIENT:
    {
        listClientFiles();
        break;
    }
    case InterfaceCommand::GET_SYNC_DIR:
    {
        synchronize();
        break;
    }
    case InterfaceCommand::EXIT:
    {
        endClient();
        break;
    }
    default:
        Logger::log("Unknown command received from the interface: " + std::to_string(+buffer[1]));
        break;
    }
}

void printFileAndMACTimes(const std::filesystem::path &filepath)
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

    std::cout << "Filename: " << filepath.filename() << std::endl;
    std::cout << "    Last modified: " << mtime << std::endl;
    std::cout << "    Last accessed: " << atime << std::endl;
    std::cout << "          Created: " << ctime << std::endl;
}

void ClientDaemon::listClientFiles()
{
    Logger::log("Listing files in sync_dir");
    std::cout << "Files in sync_dir:" << std::endl;

    for (const auto &entry : std::filesystem::directory_iterator(syncDir))
    {
        printFileAndMACTimes(entry.path());
    }
    // If no file in directory
    if (std::filesystem::directory_iterator(syncDir) == std::filesystem::directory_iterator())
    {
        std::cout << "No files in sync_dir" << std::endl;
    }
}

void ClientDaemon::listServerFiles()
{
    Logger::log("Requesting list of files from server");
    ListClientCommandMsg msg;
    char *buffer = msg.encode();
    buffer[0] = LIST_SERVER_FILES_MSG;
    serverSocket->write(buffer);
    delete[] buffer;
}

void ClientDaemon::downloadFile(const std::string filename)
{
    Logger::log("Attempting to download file from command: " + filename);
    std::string fullFilename = "./" + filename;
    FileHandlerMessage msg(0, fullFilename.c_str(), 0, NULL);
    char *buffer = msg.encode();
    buffer[0] = FILE_DOWNLOAD_MSG;
    serverSocket->write(buffer);
    delete[] buffer;
}

void ClientDaemon::uploadFile(const std::string &path)
{
    std::string filename = filenameFromPath(path);
    Logger::log("Attempting to upload file from command: " + path);
    std::cout << "Uploading file: " << path << std::endl;
    std::string filepath = std::string(path);
    FileReader *fileReader = new FileReader(username, 0, fileMutexes.getOrAddMutex(filename), filename, &socketClient, filepath);

    // fileReaders[filename] = fileReader;
    fileReader->start();
}

void ClientDaemon::deleteFile(const std::string &filename)
{
    Logger::log("Attempting to delete file from command: " + filename);
    FileDeleter deleter = FileDeleter(username, 0, fileMutexes.getOrAddMutex(filename), filename);
    deleter.start();
    deleter.join();
}

void ClientDaemon::synchronize()
{
    Logger::log("Sending sync all message to server");
    SyncAllMsg msg = SyncAllMsg();
    char *buffer = msg.encode();
    serverSocket->write(buffer);
    delete[] buffer;
}

void ClientDaemon::processListServerFilesMsg(const char *buffer)
{
    Logger::log("Received list of files from server");
    ListServerCommandMsg msg;
    msg.decode(buffer);
    std::cout << msg.data << std::endl;
}