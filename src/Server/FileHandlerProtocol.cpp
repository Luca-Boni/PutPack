#include "Server/FileHandlerProtocol.hpp"

char *FileHandlerProtocol::encode(const char *filename, const char *buffer, const int size)
{
    char *msg_buffer = new char[SOCKET_BUFFER_SIZE]();

    msg_buffer[0] = FILE_HANDLER_MSG;
    memcpy(&msg_buffer[sizeof(char)], &size, sizeof(int));
    if (filename != NULL)
        memcpy(&msg_buffer[sizeof(char) + sizeof(int)], filename, FILENAME_SIZE);
    if (buffer != NULL)
        memcpy(&msg_buffer[sizeof(char) + sizeof(int) + FILENAME_SIZE], buffer, FILE_BUFFER_SIZE);

    return msg_buffer;
}