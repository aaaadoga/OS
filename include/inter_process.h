#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#define BUFSIZE 1024

#define DISK_MANAGE_PIPE TEXT("\\\\.\\pipe\\disk_manage_pipe")

#define DM_FORMAT 0
#define DM_CREATEFILE 1
#define DM_CREATEDIR 2
#define DM_DELETEINODE 3
#define DM_LOADFILE 4
#define DM_READDIR 5
#define DM_SWAPBLOCK 6

struct DM_createFile
{
    uint16_t dir_addr;
    const char name[8];
    int size;
    char buffer[720];
};

struct DM_createDir
{
    uint16_t dir_addr;
    char name[8];
};

struct DM_deleteInode
{
    uint16_t dir_addr;
    uint16_t addr;
};

struct DM_loadFile{
    uint16_t addr;
};