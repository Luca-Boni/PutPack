#include "Server/ClientManager.hpp"
#include "Server/ClientUserProtocol.hpp"
#include "Utils/FileHandlerProtocol.hpp"
#include <iostream>

ClientManager::ClientManager(const std::string username, const unsigned long long clientId, SocketServerSession *socket, SocketClient *userManagerSocket, SocketClient *serverDaemonSocket) : Thread(std::bind(&ClientManager::execute, this, std::placeholders::_1), NULL),
                                                                                                                                                                                              username(username),
                                                                                                                                                                                              shouldStop(false),
                                                                                                                                                                                              clientId(clientId),
                                                                                                                                                                                              socket(socket),
                                                                                                                                                                                              userManagerSocket(userManagerSocket),
                                                                                                                                                                                              serverDaemonSocket(serverDaemonSocket)
{
    filesBeingEdited = std::unordered_set<std::string>();
}

void *ClientManager::execute(void *dummy)
{
    NewClientMsg newClientMsg = NewClientMsg(clientId, socket);
    char *buffer = newClientMsg.encode();
    userManagerSocket->write(buffer);
    delete[] buffer;

    while (!shouldStop || (filesBeingEdited.size() > 0))
    {
        char *buffer = socket->read();

        switch (buffer[0])
        {
        case SYNC_MSG: // Mensagem de sync_dir
            processSyncAllMsg(buffer);
            break;
        case FILE_DELETE_MSG: // Deleta um arquivo do servidor
            processFileDeleteMsg(buffer);
            break;
        case FILE_WRITE_MSG: // Escreve um arquivo no servidor
            processFileWriteMsg(buffer);
            break;
        case END_CLIENT_MSG: // Para a thread e avisa o UserManager
            processEndClientMsg(buffer);
            break;
        case FILE_UPLOAD_MSG: // Envia um arquivo para o servidor -> deve sincronizar em todos os clientes
            processFileUploadMsg(buffer);
            break;
        case FILE_DOWNLOAD_MSG: // Envia um arquivo para o cliente por demanda
            processFileDownloadMsg(buffer);
            break;
        case FILE_DEL_CMD_MSG: // Envia um comando para deletar um arquivo
            processFileDelCmdMsg(buffer);
            break;
        case LIST_SERVER_FILES_MSG:
            processListServerFilesMsg(buffer);
        default:
            Logger::log("Unknown message received: " + +buffer[0]);
            break;
        }

        delete[] buffer;
    }

    return NULL;
}

void ClientManager::processSyncAllMsg(const char *buffer)
{
    SyncAllMsg msg;
    msg.decode(buffer);
    Logger::log("Syncing all files: " + username + " " + std::to_string(clientId));
    msg.clientId = clientId;
    char *newBuffer = msg.encode();
    userManagerSocket->write(newBuffer);
    delete[] newBuffer;
}

void ClientManager::processFileDeleteMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    Logger::log("Deleting file: " + std::string(msg.filename) + " " + username + " " + std::to_string(clientId));
    msg.clientId = clientId;
    char *newBuffer = msg.encode();
    newBuffer[0] = FILE_DELETE_MSG;
    userManagerSocket->write(newBuffer);
    delete[] newBuffer;
}

void ClientManager::processFileWriteMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    Logger::log("Writing file: " + std::string(msg.filename) + " " + std::to_string(msg.size) + " bytes" + "; " + username + " " + std::to_string(clientId));

    if (msg.size == 0)
    {
        filesBeingEdited.erase(msg.filename);
    }
    else
    {
        filesBeingEdited.insert(msg.filename);
    }

    msg.clientId = clientId;
    char *newBuffer = msg.encode();
    newBuffer[0] = FILE_WRITE_MSG;
    userManagerSocket->write(newBuffer);
    delete[] newBuffer;
}

void ClientManager::processEndClientMsg(const char *buffer)
{
    EndClientMsg msg;
    msg.decode(buffer);
    Logger::log("ClientManager: Client disconnected: " + username + " " + std::to_string(clientId));
    msg.clientId = clientId;
    char *newBuffer = msg.encode();
    userManagerSocket->write(newBuffer);
    serverDaemonSocket->write(newBuffer);
    delete[] newBuffer;
    shouldStop = true;
}

void ClientManager::processFileUploadMsg(char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    Logger::log("Receiving uploaded file: " + std::string(msg.filename) + " " + std::to_string(msg.size) + " bytes" + "; " + username + " " + std::to_string(clientId));

    if (msg.size == 0)
    {
        filesBeingEdited.erase(msg.filename);
    }
    else
    {
        filesBeingEdited.insert(msg.filename);
    }

    buffer[0] = FILE_WRITE_MSG;
    userManagerSocket->write(buffer); // Não identificamos o ClientManager, dessa forma atualiza para todos
}

void ClientManager::processFileDelCmdMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    Logger::log("Deleting file: " + std::string(msg.filename) + "; " + username + " " + std::to_string(clientId));
    msg.clientId = 0; // Comando de deletar arquivo não tem clientId -> deleta para todos clientes
    char *newBuffer = msg.encode();
    newBuffer[0] = FILE_DELETE_MSG;
    userManagerSocket->write(newBuffer);
    delete[] newBuffer;
}

void ClientManager::processFileDownloadMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
    Logger::log("Sending file as download: " + std::string(msg.filename) + "; " + username + " " + std::to_string(clientId));
    msg.clientId = clientId;
    char *newBuffer = msg.encode();
    newBuffer[0] = FILE_DOWNLOAD_MSG;
    userManagerSocket->write(newBuffer);
    delete[] newBuffer;
}

void ClientManager::processListServerFilesMsg(const char *buffer)
{
    ListServerCommandMsg msg;
    msg.decode(buffer);
    Logger::log("Listing server files: " + username + " " + std::to_string(clientId));
    msg.clientId = clientId;
    char* newBuffer = msg.encode();
    userManagerSocket->write(newBuffer);
    delete[] newBuffer;
}