#include "Utils/FileHandlerProtocol.hpp"
#include <iostream>

FileHandlerMessage::FileHandlerMessage()
{
    this->clientId = 0;
    this->filename = new char[FILENAME_SIZE]();
    this->size = 0;
    this->data = new char[FILE_BUFFER_SIZE]();
}

FileHandlerMessage::FileHandlerMessage(unsigned long long clientId, const char *filename, unsigned int size, const char *data)
{
    this->clientId = clientId;
    this->filename = new char[FILENAME_SIZE]();
    this->size = size;
    this->data = new char[FILE_BUFFER_SIZE]();

    if (filename != NULL)
        memcpy(this->filename, filename, FILENAME_SIZE);

    if (data != NULL)
        memcpy(this->data, data, FILE_BUFFER_SIZE);
}

char *FileHandlerMessage::encode()
{
    char *msg_buffer = new char[SOCKET_BUFFER_SIZE]();
    int pos = 0;

    msg_buffer[pos] = FILE_READ_MSG;
    pos += sizeof(char);

    memcpy(&msg_buffer[pos], &clientId, sizeof(unsigned long long));
    pos += sizeof(unsigned long long);

    if (filename != NULL)
    {
        std::string filenameStr(filename);
        strcpy(&msg_buffer[pos], filenameStr.substr(0, FILENAME_SIZE).c_str());
    }
    pos += FILENAME_SIZE;

    memcpy(&msg_buffer[pos], &size, sizeof(unsigned int));
    pos += sizeof(unsigned int);

    if (data != NULL)
        memcpy(&msg_buffer[pos], data, FILE_BUFFER_SIZE);
    pos += FILE_BUFFER_SIZE;

    return msg_buffer;
}

FileHandlerMessage::~FileHandlerMessage()
{
    // TODO: Fix double free
    // delete[] filename;
    // delete[] data;
}

void FileHandlerMessage::decode(const char *buffer)
{
    int pos = sizeof(char);

    clientId = *(unsigned long long *)(buffer + pos);
    pos += sizeof(unsigned long long);

    memcpy(filename, buffer + pos, FILENAME_SIZE);
    pos += FILENAME_SIZE;

    size = *(unsigned int *)(buffer + pos);
    pos += sizeof(unsigned int);

    memcpy(data, buffer + pos, FILE_BUFFER_SIZE);
}