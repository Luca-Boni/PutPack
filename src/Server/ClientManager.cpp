#include "Server/ClientManager.hpp"
#include "ClientUserProtocol.hpp"
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

    while (!shouldStop && (filesBeingEdited.size() > 0))
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
        default:
            std::cerr << "Unknown message received: " << +buffer[0] << std::endl;
            break;
        }

        delete[] buffer;
    }
}

void ClientManager::processSyncAllMsg(const char *buffer)
{
    SyncAllMsg msg;
    msg.decode(buffer);
    msg.clientId = clientId;
    char *newBuffer = msg.encode();
    userManagerSocket->write(newBuffer);
    delete[] newBuffer;
}

void ClientManager::processFileDeleteMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);
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
    msg.clientId = clientId;
    char *newBuffer = msg.encode();
    userManagerSocket->write(newBuffer);
    serverDaemonSocket->write(newBuffer);
    delete[] newBuffer;
    shouldStop = true;
}

void ClientManager::processFileUploadMsg(const char *buffer)
{
    FileHandlerMessage msg;
    msg.decode(buffer);

    if (msg.size == 0)
    {
        filesBeingEdited.erase(msg.filename);
    }
    else
    {
        filesBeingEdited.insert(msg.filename);
    }
    
    userManagerSocket->write(buffer); // Não identificamos o ClientManager, dessa forma atualiza para todos
}