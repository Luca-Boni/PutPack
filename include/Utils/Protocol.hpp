#pragma once

#define STOP_MSG         (unsigned char)1
#define FILE_MONITOR_MSG (unsigned char)2
#define FILE_READ_MSG    (unsigned char)3
#define FILE_WRITE_MSG   (unsigned char)4
#define SYNC_MSG         (unsigned char)5
#define NEW_CLIENT_MSG   (unsigned char)6
#define END_CLIENT_MSG   (unsigned char)7
#define FILE_DELETE_MSG  (unsigned char)8

struct SyncAllMsg
{
    unsigned long long clientId;

    SyncAllMsg(unsigned long long clientId) : clientId(clientId) {}
    SyncAllMsg() : clientId(0) {}
    char* encode();
    void decode(const char* buffer);
};