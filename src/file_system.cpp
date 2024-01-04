#include <file_system.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <windows.h>
using namespace std;

int main()
{

    


    // 获取命令行参数
    LPSTR commandLine = GetCommandLineA();
    string str(commandLine);
    istringstream iss(str);

    // 分割字符串并输出
    string token;
    if (iss >> token) // 会直接忽略空格
    {
        if (token == "1")
        {
        }
        else if (token == "2")
        {
        }
        else if (token == "3")
        {
        }
        else if (token == "4")
        {
            iss >> token;
            char dir_name[8];
            token.copy(dir_name, sizeof(dir_name));
            dir_name[token.size()] = '\0';
            MakeDir(dir_name);
        }
        else if (token == "5")
        {
            iss >> token;
            char dir_name[8];
            token.copy(dir_name, sizeof(dir_name));
            dir_name[token.size()] = '\0';
            DeleteDir(dir_name);
        }
    }
    while (iss >> token)
    {
        std::cout << "Token: " << token << std::endl;
    }
}

int CorrespondWithCreateProcess(int size, char *data, char *file_path, char *filename)
{
    WriteFile(size, data, file_path, filename);
}

int CorrespondWithDeleteProcess(char *file_path)
{
    DeleteFile(file_path);
}

int CorrespondWithExecuteProcess(char *file_path, int memory_block_num)
{
    ReadFile(file_path);
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

FCB *CreateFCB(char *filename, bool is_dir, int file_inode_addr = -1)
{
    FCB *fcb = new FCB;
    strcpy(fcb->filename, filename);
    fcb->is_dir = is_dir;
    fcb->quote_count = 0;
    fcb->size = 0;
    fcb->file_inode_addr = -1;
    return fcb;
}

// 获取当前目录的全部数据
void MaintainFileSystemTree()
{
    // 根据inode数据构建FCB和Filenode
    // 更新已创建的节点
}

int GetRoot()
{
    // 进程通信，查询【0】号磁盘块有没有根目录数据，获取返回的0或1
}

void InitializeRoot()
{
    if (!GetRoot())
    {
        CreateRoot();
    }
    else
    {
        // 进程通信，传输【0】给磁盘管理进程，获取根目录的数据(传回来的数据是什么格式的？？？？？)

        // 将数据传给MaintainFileSystemTree
    }
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
    cout << "filesystem:" << path << ">";
    cin >> command;
    if (strcmp(command, "mkdir") == 0)
        // MakeDir();
    else if (strcmp(command, "rm") == 0)
        // DeleteDir();
    else if (strcmp(command, "cd") == 0)
        CD();
    else
        cout << "请输入正确命令!" << endl;
    return 1;
}

int MakeDir(char *dir_name)
{
    temp = InitializeNode(" ", 1);
    cin >> temp->filename;
    tempFCB = CreateFCB(temp->filename, 1);
    temp->fcb = tempFCB;
    // 进程通信将【当前目录的inode_addr】(recent->fcb->inode_addr)和【新建目录的FCB】(tempFCB)传输给磁盘管理进程
    // 进程通信获取传输回来的【新建目录的inode_addr】(temp->fcb->file_inode_addr)
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

int DeleteDir(char *dir_name)
{
    char filename[8];
    cin >> filename;
    temp = new Filenode;
    if (recent->child)
    {
        temp = recent->child;
        while (temp->next &&
               (strcmp(temp->filename, filename) != 0 || temp->is_dir != 1))
            temp = temp->next;
        if (strcmp(temp->filename, filename) != 0 || temp->is_dir != 1)
        {
            cout << "不存在该目录！" << endl;
            return 0;
        }
    }
    else
    {
        cout << "不存在该目录！" << endl;
        return 0;
    }

    if (temp->child)
    {
        cout << "目录非空，无法删除！" << endl;
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
    // 进程通信，将【当前进程的inode_addr】(recent->file_inode_addr)
    // 和【需要删除的目录的inode_addr】（temp->fcb->file_inode_addr）传给磁盘管理进程
    delete temp;
    cout << "目录已删除!" << endl;
    return 0;
}

int DeleteFile(char *file_path)
{
    char filename[8];
    char *last_slash_pos = strrchr(file_path, '/');
    if (last_slash_pos != nullptr)
    {
        strcpy(filename, last_slash_pos + 1);
    }
    else
    {
        strcpy(filename, file_path);
    }
    temp = new Filenode;

    if (recent->child)
    {
        temp = recent->child;
        while (temp->next &&
               (strcmp(temp->filename, filename) != 0 || temp->is_dir != 0))
            temp = temp->next;
        if (strcmp(temp->filename, filename) != 0 || temp->is_dir != 0)
        {
            cout << "不存在该文件！" << endl;
            return 0;
        }
    }
    else
    {
        cout << "不存在该文件！" << endl;
        return 0;
    }

    if (!temp->fcb->quote_count)
    {
        cout << "有进程打开文件，无法删除" << endl;
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
    // 进程通信，将【当前进程的inode_addr】(recent->file_inode_addr)、
    // 和【需要删除的文件的inode_addr】（temp->fcb->file_inode_addr）传给磁盘管理进程
    delete temp;
    cout << "文件已删除!" << endl;
    return 0;
}

// 这里的参数【文件路径】是相对路径，从当前目录开始
int ReadFile(char *file_path)
{
    char filename[8];
    char *last_slash_pos = strrchr(file_path, '/');
    if (last_slash_pos != nullptr)
    {
        strcpy(filename, last_slash_pos + 1);
    }
    else
    {
        strcpy(filename, file_path);
    }
    if (recent->child == NULL)
    {
        cout << "文件不存在!" << endl;
        return 1;
    }
    if (strcmp(recent->child->filename, filename) == 0)
    {
        // 进程通信，将【recent->child->fcb->file_inode_addr】传给磁盘管理进程
        return 0;
    }
    else
    {
        temp = recent->child;
        while (temp->next)
        {
            if (strcmp(temp->next->filename, filename) == 0)
            {
                // 进程通信，将【recent->child->fcb->file_inode_addr】传给磁盘管理进程
                return 0;
            }
        }
        cout << "文件不存在!" << endl;
    }
}

int WriteFile(int size, char *data, char *file_path, char *filename)
{
    // 根据路径找到最终的目录，设置成recent
    findpath(file_path);
    temp = InitializeNode(" ", 0);
    strcpy(temp->filename, filename);
    tempFCB = CreateFCB(temp->filename, 0);
    temp->fcb = tempFCB;
    // 进程通信将【当前目录的inode_addr】(recent->fcb->inode_addr)，【文件的数据】(data)
    // 和【新建目录的FCB】(tempFCB)传输给磁盘管理进程
    // 进程通信获取传输回来的【新建文件的inode_addr】(temp->fcb->file_inode_addr)
    if (recent->child == NULL)
    {
        temp->parent = recent;
        temp->child = NULL;
        recent->child = temp;
        temp->prev = temp->next = NULL;
        printf("文件建立成功!\n");
    }
    else
    {
        ttemp = recent->child;
        if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->is_dir == 0)
        {
            {
                printf("文件已存在!\n");
                return 1;
            }
        }
        while (ttemp->next)
        {
            ttemp = ttemp->next;
            if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->is_dir == 0)
            {
                printf("文件已存在!\n");
                return 1;
            }
        }
        ttemp->next = temp;
        temp->parent = NULL;
        temp->child = NULL;
        temp->prev = ttemp;
        temp->next = NULL;
        printf("文件建立成功!\n");
    }
    return 0;
}

int CD()
{
    char topath[PATH_LENGTH];
    cin >> topath;
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
    }
    else
    {
        findpath(topath);
    }
}

int findpath(char *file_path)
{
    unsigned int i = 0;
    if (strcmp(file_path, "/") == 0) // 如果命令是cd /
    {
        recent = root;
        strcpy(path, "/");
        return 0;
    }
    temp = recent;
    strcpy(temppath, path);
    if (file_path[0] == '/') // cd命令以cd /开始
    {
        recent = root->child;
        i++;
        strcpy(path, "/");
        //	printf("\n%s",path);
    }
    else
    {
        if (recent != NULL && recent != root)
        {
            strcat(path, "/");
            //  printf("\n%s\n",path);
        }

        if (recent && recent->child)
        {
            if (recent->is_dir)
                recent = recent->child;
            else
            {
                printf("路径错误！\n");
                return 1;
            }
        }
    }
    while (i <= strlen(file_path) && recent)
    {
        int j = 0;
        if (file_path[i] == '/' && recent->child)
        {
            i++;
            if (recent->is_dir)
                recent = recent->child;
            else
            {
                printf("路径错误\n");
                return 1;
            }
            strcat(path, "/");
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
        }
        if (strcmp(recent->filename, recentpath) == 0)
        {
            if (recent->is_dir == 0)
            {
                strcpy(path, temppath);
                recent = temp;
                printf("是文件不是目录。\n");
                return 1;
            }
            strcat(path, recent->filename);
        }
        if (strcmp(recent->filename, recentpath) != 0 || recent == NULL)
        {
            strcpy(path, temppath);
            recent = temp;
            printf("输入路径错误\n");
            return 1;
        }
    }
    return 0;
}