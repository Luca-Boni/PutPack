#include "Client/ClientMenu.hpp"
#include "Client/ClientInterfaceProtocol.hpp"

#include <iostream>
#include <string>

using namespace std;

void ClientMenu::start()
{
    std::string command = "";

    cout << "PutPack started. Type 'help' for a list of commands." << endl;

    while (command != "exit")
    {
        cin >> command;

        if (command == "help")
        {
            help();
        }
        else if (command == "upload")
        {
            upload();
        }
        else if (command == "download")
        {
            download();
        }
        else if (command == "delete")
        {
            deleteFile();
        }
        else if (command == "list_server")
        {
            listServerFiles();
        }
        else if (command == "list_client")
        {
            listClientFiles();
        }
        else if (command == "get_sync_dir")
        {
            synchronize();
        }
        else if (command == "exit")
        {
            exit();
        }
        else
        {
            cout << "Invalid command. Type 'help' for a list of commands." << endl;
        }
    }
}

void ClientMenu::help()
{
    cout << "Commands:" << endl;
    cout << "upload - Upload a file to the server - Usage: upload <filename>" << endl;
    cout << "download - Download a file from the server to current dir - Usage: download <filename>" << endl;
    cout << "delete - Delete a file from the server - Usage: delete <filename>" << endl;
    cout << "list_server - List all files in the server" << endl;
    cout << "list_client - List all files in the client" << endl;
    cout << "get_sync_dir - Synchronize client files with server files" << endl;
    cout << "exit - Exit the program" << endl;
}

void ClientMenu::upload()
{
    std::string filepath;
    cin >> filepath;

    if (filepath.empty())
    {
        cout << "Invalid filename. Usage: upload <filename>" << endl;
        return;
    }

    cout << "Uploading file " << filepath << "..." << endl;

    UploadCommandMsg uploadMsg(filepath);
    char *buffer = uploadMsg.encode();
    clientDaemonSocket->write(buffer);
    delete[] buffer;
}

void ClientMenu::download()
{
    std::string filename;
    cin >> filename;

    if (filename.empty())
    {
        cout << "Invalid filename. Usage: download <filename>" << endl;
        return;
    }

    cout << "Downloading file " << filename << "..." << endl;

    DownloadCommandMsg downloadMsg(filename);
    char *buffer = downloadMsg.encode();
    clientDaemonSocket->write(buffer);
    delete[] buffer;
}

void ClientMenu::deleteFile()
{
    std::string filename;
    cin >> filename;

    if (filename.empty())
    {
        cout << "Invalid filename. Usage: delete <filename>" << endl;
        return;
    }

    cout << "Deleting file " << filename << "..." << endl;

    DeleteCommandMsg deleteMsg(filename);
    char *buffer = deleteMsg.encode();
    clientDaemonSocket->write(buffer);
    delete[] buffer;
}

void ClientMenu::listServerFiles()
{
    ListClientCommandMsg listMsg;
    char *buffer = listMsg.encode();
    clientDaemonSocket->write(buffer);
    delete[] buffer;
}

void ClientMenu::listClientFiles()
{
    cout << "Listing client files..." << endl;

    ListClientCommandMsg listMsg;
    char *buffer = listMsg.encode();
    clientDaemonSocket->write(buffer);
    delete[] buffer;
}

void ClientMenu::synchronize()
{
    cout << "Synchronizing client files with server files..." << endl;

    GetSyncDirCommandMsg syncMsg;
    char *buffer = syncMsg.encode();
    clientDaemonSocket->write(buffer);
    delete[] buffer;
}

void ClientMenu::exit()
{
    cout << "Stopping client graciously..." << endl;
    ExitCommandMsg exitMsg;
    char *buffer = exitMsg.encode();
    clientDaemonSocket->write(buffer);
    delete[] buffer;
}