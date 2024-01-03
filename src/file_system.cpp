#include <file_system.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

int main()
{
    if (!GetRoot())  //进程通信，查询【0】号磁盘块有没有根目录数据，获取返回的0或1
    {
        CreateRoot();
    }
    else
    {
        // 进程通信，传输【0】给磁盘管理进程，获取根目录的数据(传回来的数据是什么格式的？？？？？)
    }
    while (1)
    {
        if (!Run())
            break;
    }
}

Filenode *InitializeNode(char *filename, bool is_dir)
{
    Filenode *node = new Filenode;
    strcpy(node->filename, filename);
    node->is_dir = is_dir;
    node->fcb = NULL;
    node->parent = NULL;
    node->child = NULL;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

FCB *CreateFCB(char *filename, bool is_dir, int file_inode_addr=-1)
{
    FCB *fcb = new FCB;
    strcpy(fcb->filename, filename);
    fcb->is_dir = is_dir;
    fcb->quote_count = 0;
    fcb->size = 0;
    fcb->file_inode_addr = -1;
    return fcb;
}

void CreateRoot()
{
    recent = root = InitializeNode("/", 1);
    root->parent = NULL;
    root->child = NULL;
    root->prev = root->next = NULL;
    root->fcb = CreateFCB("/", 1, 0);
    strcpy(path, "/");
}

int Run()
{
    cin >> command;
    if (strcmp(command, "mkdir") == 0)
        MakeDir();
    else if (strcmp(command, "rm") == 0)
        DeleteDir();
    else
        cout << "请输入正确命令!" << endl;
    return 1;
}

int MakeDir()
{
    temp = InitializeNode(" ", 1);
    cin >> temp->filename;
    tempFCB = CreateFCB(temp->filename, 1);
    // 进程通信将【当前目录的inode_addr】和【新建目录的FCB】传输给磁盘管理进程
    // 进程通信获取传输回来的【新建目录的inode_addr】
    if (recent->child == NULL)
    {
        temp->parent = recent;
        temp->child = NULL;
        recent->child = temp;
        temp->prev = temp->next = NULL;
        printf("目录建立成功!\n");
    }
    else
    {
        ttemp = recent->child;
        if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->is_dir == 1)
        {
            {
                printf("目录已存在!\n");
                return 1;
            }
        }
        while (ttemp->next)
        {
            ttemp = ttemp->next;
            if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->is_dir == 1)
            {
                printf("目录已存在!\n");
                return 1;
            }
        }
        ttemp->next = temp;
        temp->parent = NULL;
        temp->child = NULL;
        temp->prev = ttemp;
        temp->next = NULL;
        printf("目录建立成功!\n");
    }
    return 0;
}