#pragma once

#include <string>
#include <vector>

std::string filenameFromPath(const std::string &path);

enum class InterfaceCommand : unsigned char
{
    UPLOAD,
    DOWNLOAD,
    DELETE,
    LIST_SERVER,
    LIST_CLIENT,
    GET_SYNC_DIR,
    EXIT
};

class InterfaceCommandMsg
{
public:
    InterfaceCommand command;

    InterfaceCommandMsg() {}
    InterfaceCommandMsg(InterfaceCommand command) : command(command) {}
    char *encode();
    void decode(const char *buffer);
};

class FileChangeCommandMsg : public InterfaceCommandMsg
{
public:
    std::string path;
    std::string filename;

    FileChangeCommandMsg() {}
    FileChangeCommandMsg(InterfaceCommand command, std::string path, std::string filename) : InterfaceCommandMsg(command), path(path), filename(filename) {}
    char *encode();
    void decode(const char *buffer);
};

class UploadCommandMsg : public FileChangeCommandMsg
{
public:
    UploadCommandMsg() {}
    UploadCommandMsg(std::string path) : FileChangeCommandMsg(InterfaceCommand::UPLOAD, path, filenameFromPath(path)) {}
};

class DownloadCommandMsg : public FileChangeCommandMsg
{
public:
    DownloadCommandMsg() {}
    DownloadCommandMsg(std::string filename) : FileChangeCommandMsg(InterfaceCommand::DOWNLOAD, "./" + filename, filename) {}
};

class DeleteCommandMsg : public FileChangeCommandMsg
{
public:
    DeleteCommandMsg() {}
    DeleteCommandMsg(std::string filename) : FileChangeCommandMsg(InterfaceCommand::DELETE, "", filename) {}
};

class ListClientCommandMsg : public InterfaceCommandMsg
{
public:
    ListClientCommandMsg() : InterfaceCommandMsg(InterfaceCommand::LIST_CLIENT) {}
};

class GetSyncDirCommandMsg : public InterfaceCommandMsg
{
public:
    GetSyncDirCommandMsg() : InterfaceCommandMsg(InterfaceCommand::GET_SYNC_DIR) {}
};

class ExitCommandMsg : public InterfaceCommandMsg
{
public:
    ExitCommandMsg() : InterfaceCommandMsg(InterfaceCommand::EXIT) {}
};