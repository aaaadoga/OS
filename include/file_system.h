#include <bits/stdc++.h>

#define COMMAND_LENGTH 10 // 命令行长度
#define PATH_LENGTH 30    // 参数长度

struct FCB
{
    char filename[8];
    int quote_count;
    bool is_dir;
    int size;
    int file_inode_addr;
};

struct Filenode
{
    char filename[8];
    bool is_dir;
    FCB *fcb;
    Filenode *parent;
    Filenode *child;
    Filenode *next;
    Filenode *prev;
};



struct DirData
{
    int mode;
    char filename[8];
    int size;
    int inode_addr;
};

Filenode *root,
    *recent, *temp, *ttemp, *temp_child;
char path[PATH_LENGTH], command[COMMAND_LENGTH], temppath[PATH_LENGTH],
    recentpath[PATH_LENGTH];
FCB *tempFCB;
int execute_process_initial_addr = -1;
DirData dir_data[100];

Filenode *InitializeNode(char *filename, bool is_dir);

void CreateRoot();

FCB *CreateFCB(char *filename, bool is_dir, int file_inode_addr = -1);

void MaintainFileSystemTree();

int WriteFile(int size, char *data, char *file_path, char *filename);

int FindPath(char *file_path);

int CD(char *topath, uint16_t *path_len);

int ReadFile(char *file_path);

int MyDeleteFile(char *file_path);

int MakeDir(char *dir_name);

int DeleteDir(char *dir_name);

int ShowDir(char *dir_info);
