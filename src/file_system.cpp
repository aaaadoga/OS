#include <file_system.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <windows.h>
#include <inter_process.h>
#include <sstream>

using namespace std;

DWORD WINAPI InstanceThread(LPVOID lpvParam);
int Client(LPTSTR pipe_name, uint8_t *req, DWORD req_size, uint8_t *res, DWORD *res_size, bool ret);
VOID GetAnswerToRequest(uint8_t *pchRequest, DWORD cbBytesRead,
                        uint8_t *pchReply,
                        LPDWORD pchBytes);
int main()
{

    CreateRoot();
    ////////////////////////////////////////////////////////////////////////////////////////////
    // 开启服务器

    BOOL fConnected = FALSE;
    DWORD dwThreadId = 0;
    HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
    LPCTSTR lpszPipename = FILE_MANAGE_PIPE;

    // 主循环创建了一个命名管道的实例然后等待一个客户端连接它。
    // 当客户端连接时，一个线程被创建来处理与那个客户端的通信
    // 这个循环可以自由的等待下一个客户端连接请求，这是一个无限循环。

    for (;;)
    {
        _tprintf(TEXT("\nFile Manage Server: Main thread awaiting client connection on %s\n"), lpszPipename);
        hPipe = CreateNamedPipe(
            lpszPipename,             // pipe name
            PIPE_ACCESS_DUPLEX,       // read/write access
            PIPE_TYPE_BYTE |          // message type pipe
                PIPE_READMODE_BYTE |  // message-read mode
                PIPE_WAIT,            // blocking mode
            PIPE_UNLIMITED_INSTANCES, // max. instances
            BUFSIZE,                  // output buffer size
            BUFSIZE,                  // input buffer size
            0,                        // client time-out
            NULL);                    // default security attribute

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            _tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
            return -1;
        }

        // 阻塞等待客户端连接，如果成功连接
        // 函数返回一个非零值，如果函数
        // 返回0，则错误。

        fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (fConnected) // 成功连接
        {
            printf("Client connected, creating a processing thread.\n");

            // 为这个客户端创建一个线程
            hThread = CreateThread(
                NULL,           // no security attribute
                0,              // 默认栈大小
                InstanceThread, // 线程程序
                (LPVOID)hPipe,  // 线程参数
                0,              // 不挂起
                &dwThreadId);   // 返回的线程的ID

            if (hThread == NULL)
            {
                _tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
                return -1;
            }
            else
                CloseHandle(hThread);
        }
        else
            // The client could not connect, so close the pipe.
            CloseHandle(hPipe);
    }
    return 0;
}

Filenode *InitializeNode(char *filename, bool is_dir)
{
    Filenode *node = new Filenode;
    memset(node, 0, sizeof(node));
    memcpy(node->filename, filename, strlen(filename));
    node->is_dir = is_dir;
    node->fcb = NULL;
    node->parent = NULL;
    node->child = NULL;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

FCB *CreateFCB(char *filename, bool is_dir, int file_inode_addr)
{
    FCB *fcb = new FCB;
    memset(fcb, 0, sizeof(fcb));
    memcpy(fcb->filename, filename, strlen(filename));
    fcb->is_dir = is_dir;
    fcb->quote_count = 0;
    fcb->size = 0;
    fcb->file_inode_addr = file_inode_addr;
    return fcb;
}

// 获取当前目录的全部数据
void MaintainFileSystemTree()
{
    uint8_t req[1 + sizeof(DM_readDir)];
    req[0] = DM_READDIR;
    DM_readDir *p = (DM_readDir *)(req + 1);
    p->addr = recent->fcb->file_inode_addr;
    uint8_t res_data[1024];
    DWORD res_size;
    Client(DISK_MANAGE_PIPE, req, 1 + sizeof(DM_readDir), (uint8_t *)res_data, &res_size, true);
    uint16_t *file_count = (uint16_t *)(res_data);
    temp = new Filenode;
    memset(temp, 0, sizeof(temp));
    FCB_P *tempFCB_P = new FCB_P;
    for (int i = 0; i < *file_count; i++)
    {
        tempFCB_P = (FCB_P *)(res_data + 2 * (i + 1));
        memcpy(temp->filename, tempFCB_P->name, strlen(tempFCB_P->name));
        if (tempFCB_P->mode)
        {
            temp->is_dir = 1;
        }
        else
        {
            temp->is_dir = 0;
        }
        tempFCB = CreateFCB(tempFCB_P->name, temp->is_dir, tempFCB_P->addr);
        temp->fcb = tempFCB;
        temp->fcb->size = tempFCB_P->size;

        if (recent->child == NULL)
        {
            recent->child = temp;
            temp->parent = recent;
            temp->child = NULL;
            temp->next = NULL;
            temp->prev = NULL;
        }
        else
        {
            ttemp = recent->child;
            while (ttemp->next)
            {
                ttemp = ttemp->next;
            }
            ttemp->next = temp;
            temp->prev = ttemp;
            temp->parent = temp->next = temp->child = NULL;
        }
    }

    // 根据inode数据构建FCB和Filenode
    // 更新已创建的节点
}

void CreateRoot()
{
    root = InitializeNode("/", 1);
    root->parent = NULL;
    root->child = NULL;
    root->prev = root->next = NULL;
    root->fcb = CreateFCB("/", 1, 0);
    recent = root;
    memcpy(path, "/", 1);
    MaintainFileSystemTree();
}

int ShowDir(char *dir_info)
{
    std::string buffer;
    std::stringstream sstream;
    int i = 0, j = 0;
    temp = new Filenode;
    temp = recent;
    if (temp == root)
    {
        sstream << "      <DIR>                         "
                << "." << endl;
    }
    if (temp != root)
    {
        sstream << "      <DIR>                         "
                << ".." << endl;
        i++;
    }
    if (temp->child == NULL)
    {
        sstream << "Total: "
                << " directors      " << i << "        files       " << j << endl;
        buffer = sstream.str();
        memset(dir_info, 0, buffer.length() + 1);
        memcpy(dir_info, buffer.c_str(), buffer.length());
        return 1;
    }
    temp = temp->child;
    while (temp)
    {
        if (temp->is_dir)
        {
            sstream << "      <DIR>                        " << temp->filename << endl;
            i++;
        }
        else
        {
            sstream << "      <FILE>                       " << temp->filename << endl;
            j++;
        }
        temp = temp->next;
    }
    sstream << "Total: "
            << " directors      " << i << "          files          " << j << endl;
    buffer = sstream.str();
    memset(dir_info, 0, buffer.length() + 1);
    memcpy(dir_info, buffer.c_str(), buffer.length());
    return 0;
}

int MakeDir(char *dir_name)
{
    temp = InitializeNode(" ", 1);
    tempFCB = CreateFCB(temp->filename, 1);
    temp->fcb = tempFCB;
    uint8_t req[1 + sizeof(DM_createDir)] = {DM_CREATEDIR};
    DM_createDir *p = (DM_createDir *)(req + 1);
    memset(p, 0, sizeof(*p));
    p->dir_addr = recent->fcb->file_inode_addr;
    memcpy(p->name, dir_name, strlen(dir_name));
    uint16_t dir_inode_addr;
    DWORD res_size;
    Client(DISK_MANAGE_PIPE, req, 1 + sizeof(DM_createDir), (uint8_t *)&dir_inode_addr, &res_size, true);
    temp->fcb->file_inode_addr = dir_inode_addr;
    // 进程通信将【当前目录的inode_addr】(recent->fcb->inode_addr)和【新建目录的FCB】(tempFCB)传输给磁盘管理进程
    // 进程通信获取传输回来的【新建目录的inode_addr】(temp->fcb->file_inode_addr)
    if (recent->child == NULL)
    {
        temp->parent = recent;
        temp->child = NULL;
        recent->child = temp;
        temp->prev = temp->next = NULL;
        printf("create dir success!\n");
    }
    else
    {
        ttemp = recent->child;
        if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->is_dir == 1)
        {
            {
                printf("dir has already exist!\n");
                return 1;
            }
        }
        while (ttemp->next)
        {
            ttemp = ttemp->next;
            if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->is_dir == 1)
            {
                printf("dir has already exist!\n");
                return 1;
            }
        }
        ttemp->next = temp;
        temp->parent = NULL;
        temp->child = NULL;
        temp->prev = ttemp;
        temp->next = NULL;
        printf("create dir success!\n");
    }
    return 0;
}

int DeleteDir(char *dir_name)
{
    char filename[8] = {0};
    memcpy(filename, dir_name, 8);

    temp = new Filenode;
    if (recent->child)
    {
        temp = recent->child;
        while (temp->next &&
               (strcmp(temp->filename, filename) != 0 || temp->is_dir != 1))
            temp = temp->next;
        if (strcmp(temp->filename, filename) != 0 || temp->is_dir != 1)
        {
            std::cout << "dir did not exist" << endl;
            return 0;
        }
    }
    else
    {
        std::cout << "dir did not exist" << endl;
        return 0;
    }

    if (temp->child)
    {
        std::cout << "not empty dir" << endl;
        return 0;
    }

    if (temp->parent == NULL)
    {
        temp->prev->next = temp->next;
        if (temp->next)
            temp->next->prev = temp->prev;
        else
            temp->prev->next = NULL;
    }
    else
    {
        temp->parent->child = temp->next;
    }
    uint8_t req[1 + sizeof(DM_deleteInode)];
    req[0] = DM_DELETEINODE;
    DM_deleteInode *p = (DM_deleteInode *)(req + 1);
    p->dir_addr = recent->fcb->file_inode_addr;
    p->addr = temp->fcb->file_inode_addr;
    uint8_t res_data;
    DWORD res_size;
    Client(DISK_MANAGE_PIPE, req, 1 + sizeof(DM_deleteInode), (uint8_t *)res_data, &res_size, false);
    // 进程通信，将【当前进程的inode_addr】(recent->file_inode_addr)
    // 和【需要删除的目录的inode_addr】（temp->fcb->file_inode_addr）传给磁盘管理进程
    delete temp;
    std::cout << "delete dir success!" << endl;
    return 0;
}

int MyDeleteFile(char *file_path)
{
    char filename[8] = {0};
    char *last_slash_pos = strrchr(file_path, '/');
    if (last_slash_pos != nullptr)
    {
        memcpy(filename, last_slash_pos + 1, strlen(last_slash_pos + 1));
    }
    else
    {
        memcpy(filename, file_path, strlen(file_path));
    }

    temp = new Filenode;
    memset(temp, 0, sizeof(temp));

    if (recent->child)
    {
        temp = recent->child;
        while (temp->next &&
               (strcmp(temp->filename, filename) != 0 || temp->is_dir != 0))
            temp = temp->next;
        if (strcmp(temp->filename, filename) != 0 || temp->is_dir != 0)
        {
            std::cout << "file did not exist" << endl;
            return 0;
        }
    }
    else
    {
        std::cout << "file did not exist" << endl;
        return 0;
    }

    if (temp->fcb->quote_count)
    {
        std::cout << "can not delete file" << endl;
        return 0;
    }

    if (temp->parent == NULL)
    {
        temp->prev->next = temp->next;
        if (temp->next)
            temp->next->prev = temp->prev;
        else
            temp->prev->next = NULL;
    }
    else
    {
        temp->parent->child = temp->next;
    }
    uint8_t req[1 + sizeof(DM_deleteInode)];
    req[0] = DM_DELETEINODE;
    DM_deleteInode *p = (DM_deleteInode *)(req + 1);
    p->dir_addr = recent->fcb->file_inode_addr;
    p->addr = temp->fcb->file_inode_addr;
    uint8_t res_data;
    DWORD res_size;
    Client(DISK_MANAGE_PIPE, req, 1 + sizeof(DM_deleteInode), (uint8_t *)res_data, &res_size, false);
    // 进程通信，将【当前进程的inode_addr】(recent->file_inode_addr)、
    // 和【需要删除的文件的inode_addr】（temp->fcb->file_inode_addr）传给磁盘管理进程
    delete temp;
    std::cout << "delete file success!" << endl;
    return 0;
}

// 这里的参数【文件路径】是相对路径，从当前目录开始
int ReadFile(char *file_path)
{
    char filename[8];
    char *last_slash_pos = strrchr(file_path, '/');
    if (last_slash_pos != nullptr)
    {
        memcpy(filename, last_slash_pos + 1, strlen(last_slash_pos + 1));
    }
    else
    {
        memcpy(filename, file_path, strlen(file_path));
    }
    if (recent->child == NULL)
    {
        std::cout << "file did not exist!" << endl;
        return 1;
    }
    if (strcmp(recent->child->filename, filename) == 0)
    {
        uint8_t req[1 + sizeof(DM_loadToMemory)];
        req[0] = DM_LOADTOMEMORY;
        DM_loadToMemory *p = (DM_loadToMemory *)(req + 1);
        p->inode_addr = recent->child->fcb->file_inode_addr;
        p->index = execute_process_initial_addr;
        uint8_t res_data;
        DWORD res_size;
        Client(DISK_MANAGE_PIPE, req, 1 + sizeof(DM_loadToMemory), (uint8_t *)res_data, &res_size, false);
        // 进程通信，将【recent->child->fcb->file_inode_addr】和【memory_index】传给磁盘管理进程
        return 0;
    }
    else
    {
        temp = recent->child;
        while (temp->next)
        {
            if (strcmp(temp->next->filename, filename) == 0)
            {
                uint8_t req[1 + sizeof(DM_loadToMemory)];
                req[0] = DM_LOADTOMEMORY;
                DM_loadToMemory *p = (DM_loadToMemory *)(req + 1);
                p->inode_addr = recent->child->fcb->file_inode_addr;
                p->index = execute_process_initial_addr;
                uint8_t res_data;
                DWORD res_size;
                Client(DISK_MANAGE_PIPE, req, 1 + sizeof(DM_loadToMemory), (uint8_t *)res_data, &res_size, false);
                // 进程通信，将【recent->child->fcb->file_inode_addr】传给磁盘管理进程
                return 0;
            }
        }
        std::cout << "file did not exist!" << endl;
    }
}

int WriteFile(int size, char *data, char *file_path, char *filename)
{
    // 根据路径找到最终的目录，设置成recent
    FindPath(file_path);
    temp = InitializeNode(" ", 0);
    memcpy(temp->filename, filename, strlen(filename));
    tempFCB = CreateFCB(temp->filename, 0);
    temp->fcb = tempFCB;

    uint8_t req[1 + sizeof(DM_createFile)];
    req[0] = DM_CREATEFILE;
    DM_createFile *p = (DM_createFile *)(req + 1);
    p->dir_addr = recent->fcb->file_inode_addr;
    memcpy(p->name, filename, strlen(filename));
    p->size = size;
    memcpy(p->buffer, data, strlen(data));
    DWORD res_size;
    uint16_t inode_addr;
    Client(DISK_MANAGE_PIPE, req, 1 + sizeof(DM_createFile), (uint8_t *)&inode_addr, &res_size,true);
    temp->fcb->file_inode_addr = inode_addr;
    temp->fcb->size = size;
    // 进程通信将【当前目录的inode_addr】(recent->fcb->inode_addr)，【文件的数据】(data)
    // 和【新建目录的FCB】(tempFCB)传输给磁盘管理进程
    // 进程通信获取传输回来的【新建文件的inode_addr】(temp->fcb->file_inode_addr)
    if (recent->child == NULL)
    {
        temp->parent = recent;
        temp->child = NULL;
        recent->child = temp;
        temp->prev = temp->next = NULL;
        printf("create file success!\n");
    }
    else
    {
        ttemp = recent->child;
        if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->is_dir == 0)
        {
            {
                printf("file has already exist!\n");
                return 1;
            }
        }
        while (ttemp->next)
        {
            ttemp = ttemp->next;
            if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->is_dir == 0)
            {
                printf("file has already exist!!\n");
                return 1;
            }
        }
        ttemp->next = temp;
        temp->parent = NULL;
        temp->child = NULL;
        temp->prev = ttemp;
        temp->next = NULL;
        printf("create file success!\n");
    }
    return 0;
}

int CD(char *topath, uint16_t *path_len)
{
    if (strcmp(topath, ".") == 0)
        return 0;
    if (strcmp(topath, "..") == 0)
    {
        int i;
        while (recent->prev)
            recent = recent->prev; // 向前回溯，找到第一次创建的目录
        if (recent->parent)
        {
            recent = recent->parent;
        }

        i = strlen(path);
        // printf("%d %s\n",i,path);
        while (path[i] != '/' && i > 0)
            i--; // 找到最右边的/
        if (i != 0)
        {
            path[i] = '\0';
            // printf("%s",path); //path中不止有一个/
        }

        else
            path[i + 1] = '\0';
        *path_len = strlen(path);
    }
    else
    {
        FindPath(topath);
        *path_len = strlen(path);
    }
}

int FindPath(char *file_path)
{
    unsigned int i = 0;
    if (strcmp(file_path, "/") == 0) // 如果命令是cd /
    {
        recent = root;
        MaintainFileSystemTree();
        memcpy(path, "/", 1);
        return 0;
    }
    temp = recent;
    memcpy(temppath, path, 1);
    if (file_path[0] == '/') // cd命令以cd /开始
    {
        recent = root->child;
        MaintainFileSystemTree();
        i++;
        memcpy(path, "/", 1);
        //	printf("\n%s",path);
    }
    else
    {
        if (recent != NULL && recent != root)
        {
            string str(path);
            str = str + "/";
            const char *temp_char = str.c_str();
            memcpy(path, temp_char, strlen(temp_char));
            //  printf("\n%s\n",path);
        }

        if (recent && recent->child)
        {
            if (recent->is_dir)
                recent = recent->child;
            MaintainFileSystemTree();
        }
        else
        {
            printf("path error\n");
            return 1;
        }
    }

    while (i <= strlen(file_path) && recent != NULL)
    {
        int j = 0;
        if (file_path[i] == '/' && recent->child)
        {
            i++;
            if (recent->is_dir)
            {
                recent = recent->child;
                MaintainFileSystemTree();
            }
            else
            {
                printf("path error\n");
                return 1;
            }
            string str(path);
            str = str + "/";
            const char *temp_char = str.c_str();
            memcpy(path, temp_char, strlen(temp_char));
        }
        while (file_path[i] != '/' && i <= strlen(file_path))
        {
            recentpath[j] = file_path[i];
            i++;
            j++;
        }
        recentpath[j] = '\0';
        while (
            (strcmp(recent->filename, recentpath) != 0 || (recent->is_dir != 1)) &&
            recent->next != NULL)
        {
            recent = recent->next;
            MaintainFileSystemTree();
        }
        if (strcmp(recent->filename, recentpath) == 0)
        {
            if (recent->is_dir == 0)
            {
                memcpy(path, temppath, strlen(temppath));
                recent = temp;
                MaintainFileSystemTree();
                printf("not dir\n");
                return 1;
            }
            string str(path);
            str = str + recent->filename;
            const char *temp_char = str.c_str();
            memcpy(path, temp_char, strlen(temp_char));
        }
        if (strcmp(recent->filename, recentpath) != 0 || recent == NULL)
        {
            memcpy(path, temppath, strlen(temppath));
            recent = temp;
            MaintainFileSystemTree();
            printf("path error\n");
            return 1;
        }
    }
    return 0;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
// 这个例程是一个线程处理函数，用于通过从主循环传递的开放管道连接读取和回复客户端
// 注意，这允许主循环继续执行，根据传入的客户端连接的数量，可能同时创建更多的
// 这个过程的线程来并发运行。
{
    HANDLE hHeap = GetProcessHeap();
    TCHAR *pchRequest = (TCHAR *)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));
    TCHAR *pchReply = (TCHAR *)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));

    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    HANDLE hPipe = NULL;

    // Do some extra error checking since the app will keep running even if this
    // thread fails.

    if (lpvParam == NULL)
    {
        printf("\nERROR - Disk Manager Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL value in lpvParam.\n");
        printf("   InstanceThread exitting.\n");
        if (pchReply != NULL)
            HeapFree(hHeap, 0, pchReply);
        if (pchRequest != NULL)
            HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    if (pchRequest == NULL)
    {
        printf("\nERROR - Disk Manager Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL heap allocation.\n");
        printf("   InstanceThread exitting.\n");
        if (pchReply != NULL)
            HeapFree(hHeap, 0, pchReply);
        return (DWORD)-1;
    }

    if (pchReply == NULL)
    {
        printf("\nERROR - Disk Manager Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL heap allocation.\n");
        printf("   InstanceThread exitting.\n");
        if (pchRequest != NULL)
            HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    // Print verbose messages. In production code, this should be for debugging only.
    printf("InstanceThread created, receiving and processing messages.\n");

    // The thread's parameter is a handle to a pipe object instance.

    hPipe = (HANDLE)lpvParam;

    // 循环直到完成读取
    while (1)
    {
        // 从管道读取客户端请求. 这个简单的代码只允许消息
        // 长度最多为 BUFSIZE 个字符。
        fSuccess = ReadFile(
            hPipe,                   // 管道句柄
            pchRequest,              // 接收信息的buffer
            BUFSIZE * sizeof(TCHAR), // buffer的大小
            &cbBytesRead,            // 读多少byte
            NULL);                   // not overlapped I/O

        if (!fSuccess || cbBytesRead == 0)
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                _tprintf(TEXT("InstanceThread: client disconnected.\n"));
            }
            else
            {
                _tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
            }
            break;
        }

        // Process the incoming message.
        GetAnswerToRequest((uint8_t *)pchRequest, cbBytesRead, (uint8_t *)pchReply, &cbReplyBytes);
        if (cbReplyBytes == 0)
        {
            break;
        }
        // Write the reply to the pipe.
        fSuccess = WriteFile(
            hPipe,        // handle to pipe
            pchReply,     // buffer to write from
            cbReplyBytes, // number of bytes to write
            &cbWritten,   // number of bytes written
            NULL);        // not overlapped I/O

        if (!fSuccess || cbReplyBytes != cbWritten)
        {
            _tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError());
            break;
        }
    }

    // Flush the pipe to allow the client to read the pipe's contents
    // before disconnecting. Then disconnect the pipe, and close the
    // handle to this pipe instance.

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    HeapFree(hHeap, 0, pchRequest);
    HeapFree(hHeap, 0, pchReply);

    printf("InstanceThread exiting.\n");
    return 1;
}

VOID GetAnswerToRequest(uint8_t *pchRequest, DWORD cbBytesRead,
                        uint8_t *pchReply,
                        LPDWORD pchBytes)
{
    uint8_t *type = (uint8_t *)(pchRequest);
    if (*type == FM_CREATE_FILE)
    {
        FM_CreateFile *p = (FM_CreateFile *)(pchRequest + 1);
        uint8_t *reply = (uint8_t *)(pchReply);
        char file_content[255] = {0};
        memcpy(file_content, p->file_content, strlen(p->file_content));
        int content_len = strlen(file_content);
        char filename[8] = {0};
        char file_path[30] = {0};
        int cnt = 0;
        for (int i = 0; i < strlen(p->file_path); i++)
        {
            if (p->file_path[i] == '/')
            {
                cnt++;
            }
        }
        if (cnt == 0)
        {
            memcpy(file_path, path, strlen(path));
        }
        else if (cnt == 1)
        {
            memcpy(file_path, "/", 1);
            char *last_slash_pos = strrchr(p->file_path, '/');
            memcpy(filename, last_slash_pos + 1, strlen(last_slash_pos + 1));
        }
        else
        {
            char *last_slash_pos = strrchr(p->file_path, '/');
            memcpy(filename, last_slash_pos + 1, strlen(last_slash_pos + 1));
            *last_slash_pos = '\0';
            memcpy(file_path, p->file_path, strlen(p->file_path) - strlen(filename));
        }

        WriteFile(content_len, file_content, file_path, filename);
        *pchBytes = 0;
    }
    else if (*type == FM_DELETE_FILE)
    {
        FM_DeleteFile *p = (FM_DeleteFile *)(pchRequest + 1);
        uint8_t *reply = (uint8_t *)(pchReply);
        MyDeleteFile(p->file_path);
        *pchBytes = 0;
    }
    else if (*type == FM_CREATE_DIR)
    {
        FM_CreateDir *p = (FM_CreateDir *)(pchRequest + 1);
        MakeDir(p->file_name);
        *pchBytes = 0;
    }
    else if (*type == FM_DELETE_DIR)
    {
        FM_DeleteDir *p = (FM_DeleteDir *)(pchRequest + 1);
        uint8_t *reply = (uint8_t *)(pchReply);
        DeleteDir(p->file_name);
        *pchBytes = 0;
    }
    else if (*type == FM_READ_FILE)
    {
        FM_ReadFile *p = (FM_ReadFile *)(pchRequest + 1);
        uint8_t *reply = (uint8_t *)(pchReply);
        execute_process_initial_addr = p->memory_block_NO;
        ReadFile(p->file_path);
        *pchBytes=0;
    }
    else if (*type == FM_CHANGE_DIR)
    {
        FM_ChangeDir *p = (FM_ChangeDir *)(pchRequest + 1);
        uint16_t path_len;
        CD(p->file_name, &path_len);
        char *path_copy = (char *)(pchReply);
        memcpy(path_copy, path, path_len);
        *pchBytes = path_len;
    }
    else if (*type == FM_SHOW_DIR)
    {
        char *dir_info = (char *)(pchReply);

        ShowDir(dir_info);
        *pchBytes = strlen(dir_info);
    }
    else
    {
        *pchBytes = 0;
    }
}

int Client(LPTSTR pipe_name, uint8_t *req, DWORD req_size, uint8_t *res, DWORD *res_size, bool ret)
{
    HANDLE hPipe;
    TCHAR chBuf[BUFSIZE];
    BOOL fSuccess = FALSE;
    DWORD cbRead, cbToWrite, cbWritten, dwMode;
    LPTSTR lpszPipename = pipe_name;

    // Try to open a named pipe; wait for it, if necessary.

    while (1)
    {
        hPipe = CreateFile(
            lpszPipename,  // pipe name
            GENERIC_READ | // read and write access
                GENERIC_WRITE,
            0,             // no sharing
            NULL,          // default security attributes
            OPEN_EXISTING, // opens existing pipe
            0,             // default attributes
            NULL);         // no template file

        // Break if the pipe handle is valid.

        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        // Exit if an error other than ERROR_PIPE_BUSY occurs.

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            _tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
            return -1;
        }

        // All pipe instances are busy, so wait for 20 seconds.

        if (!WaitNamedPipe(lpszPipename, 20000))
        {
            printf("Could not open pipe: 20 second wait timed out.");
            return -1;
        }
    }

    // The pipe connected; change to message-read mode.

    dwMode = PIPE_READMODE_BYTE;
    fSuccess = SetNamedPipeHandleState(
        hPipe,   // pipe handle
        &dwMode, // new pipe mode
        NULL,    // don't set maximum bytes
        NULL);   // don't set maximum time
    if (!fSuccess)
    {
        _tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
        return -1;
    }

    // Send a message to the pipe server.

    cbToWrite = req_size;

    fSuccess = WriteFile(
        hPipe,      // pipe handle
        req,        // message
        cbToWrite,  // message length
        &cbWritten, // bytes written
        NULL);      // not overlapped

    if (!fSuccess)
    {
        _tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
        return -1;
    }

    // printf("\nMessage sent to server, receiving reply as follows:\n");

    if (!ret)
    {
        return 0;
    }

    do
    {
        // Read from the pipe.
        uint8_t abc[1024];
        fSuccess = ReadFile(
            hPipe,                   // pipe handle
            abc,                     // buffer to receive reply
            BUFSIZE * sizeof(TCHAR), // size of buffer
            res_size,                // number of bytes read
            NULL);                   // not overlapped

        if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
            break;

        _tprintf(TEXT("\"%s\"\n"), chBuf);
    } while (!fSuccess); // repeat loop if ERROR_MORE_DATA

    if (!fSuccess)
    {
        _tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
        return -1;
    }

    CloseHandle(hPipe);

    return 0;
}