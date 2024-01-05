#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <cstdint>

#define BUFSIZE 1024

#define DISK_MANAGE_PIPE TEXT("\\\\.\\pipe\\disk_manage_pipe")
#define FILE_MANAGE_PIPE TEXT("\\\\.\\pipe\\file_manage_pipe")
#define MEMORY_MANAGE_PIPE TEXT("\\\\.\\pipe\\memory_manage_pipe")

#define DM_FORMAT 0
#define DM_CREATEFILE 1
#define DM_CREATEDIR 2
#define DM_DELETEINODE 3
#define DM_LOADFILE 4
#define DM_READDIR 5
#define DM_READBLOCK 6
#define DM_LOADTOMEMORY 7

#define MM_MALLOCMEMORY 0
#define MM_FREEMEMORY 1
#define MM_REQUESTPAGE 2
#define MM_LOADFILE 3

#define FM_CREATE_FILE 0
#define FM_DELETE_FILE 1
#define FM_READ_FILE 2
#define FM_CREATE_DIR 3
#define FM_DELETE_DIR 4
#define FM_CHANGE_DIR 5
#define FM_SHOW_DIR 6

struct DM_createFile
{
    uint16_t dir_addr;
    char name[8];
    int size; // byte
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

struct DM_loadFile
{
    uint16_t addr;
};

struct DM_readDir
{
    uint16_t addr;
};

struct DM_readBlock
{
    uint16_t addr;
};

struct DM_loadToMemory
{
    uint8_t index;
    uint16_t inode_addr;
};

struct FCB_P // 读目录
{
    uint16_t mode;
    uint16_t size;
    char name[8];
    uint16_t addr;
};

struct MM_free_memory
{
    uint8_t index;
};

struct MM_request_page
{
    uint8_t index;
    uint8_t page_id;
};

struct MM_load_file
{
    uint8_t index;
    uint8_t size;
    uint16_t inode_addr[18];
};

struct FM_CreateFile
{
    char file_path[30];
    char file_content[255];
};

struct FM_DeleteFile
{
    char file_path[30];
};

struct FM_ReadFile
{
    char file_path[30];
    uint8_t memory_block_NO;
};

struct FM_CreateDir
{
    char file_name[8];
};

struct FM_CurrentPath
{
    char file_path[30];
};

struct FM_DeleteDir
{
    char file_name[8];
};

struct FM_ChangeDir
{
    char file_name[8];
};


