#include <DiskManage.h>
#include <FileSystem.h>
#include <MemoryManage.h>
#include <ProcessManage.h>

#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

#define FILENAME_LENGTH 10 // 文件名称长度
#define COMMAND_LENGTH 10  // 命令行长度
#define PATH_LENGTH 30     // 参数长度

struct filenode {
  char filename[FILENAME_LENGTH];
  int isdir;
  char content[255];
  filenode *parent;
  filenode *child;
  filenode *prev;
  filenode *next;
};

filenode *initnode(char filename[], int isdir);
void createroot();
int run();
int findpath(char *topath);
void help();
int mkdir();
int touch();
int read();
int write();
int rm();
int rmdir();
int cd();
int ls();
filenode *root, *recent, *temp, *ttemp, *temp_child;
char path[PATH_LENGTH], command[COMMAND_LENGTH], temppath[PATH_LENGTH],
    recentpath[PATH_LENGTH];

// 创建文件或目录的存储节点
filenode *initnode(const char filename[], int isdir) {
  filenode *node = new filenode;
  strcpy(node->filename, filename);
  node->isdir = isdir;
  node->parent = NULL;
  node->child = NULL;
  node->prev = NULL;
  node->next = NULL;
  return node;
}

// 初始化文件系统根结点
void createroot() {
  recent = root = initnode("/", 1);
  root->parent = NULL;
  root->child = NULL;
  root->prev = root->next = NULL;
  strcpy(path, "/");
}

void help() {
  cout << endl;
  cout << "touch:             建立文件。                " << endl;
  cout << "read:               读取文件。                  " << endl;
  cout << "write:              写入文件。                 " << endl;
  cout << "rm:             删除文件。                  " << endl;
  cout << "rmdir:                 删除目录。                  " << endl;
  cout << "mkdir:              建立目录。                " << endl;
  cout << "cd:                 切换目录。                  " << endl;
  cout << "ls:                显示目录。                  " << endl;
  cout << "logout:             退出登录。                " << endl;
}

int ls() {
  int i = 0, j = 0;
  temp = new filenode;
  temp = recent;
  if (temp == root) {
    cout << "      <DIR>                         "
         << "." << endl;
  }
  if (temp != root) {
    cout << "      <DIR>                         "
         << ".." << endl;
    i++;
  }
  if (temp->child == NULL) {
    cout << "Total: "
         << " directors      " << i << "        files       " << j << endl;
    return 1;
  }
  temp = temp->child;
  while (temp) {
    if (temp->isdir) {
      cout << "      <DIR>                        " << temp->filename << endl;
      i++;
    } else {
      cout << "      <FILE>                       " << temp->filename << endl;
      j++;
    }
    temp = temp->next;
  }
  cout << "Total: "
       << " directors      " << i << "          files          " << j << endl;
  return 0;
}

int read() {
  char filename[FILENAME_LENGTH];
  cin >> filename;
  if (recent->child == NULL) {
    cout << "文件不存在!" << endl;
    return 1;
  }
  if (strcmp(recent->child->filename, filename) == 0) {
    cout << recent->child->content << endl;
    return 0;
  } else {
    temp = recent->child;
    while (temp->next) {
      if (strcmp(temp->next->filename, filename) == 0) {
        cout << temp->next->content << endl;
        return 0;
      }
    }
    cout << "文件不存在!" << endl;
    return 1;
  }
}

int write() {
  char filename[FILENAME_LENGTH];
  cin >> filename;
  if (recent->child == NULL) {
    cout << "文件不存在!" << endl;
    return 1;
  }
  if (strcmp(recent->child->filename, filename) == 0) {
    cin >> recent->child->content;
    cout << "文件写入成功!" << endl;
    return 0;
  } else {
    temp = recent->child;
    while (temp->next) {
      if (strcmp(temp->next->filename, filename) == 0) {
        cin >> temp->next->content;
        cout << "文件写入成功!" << endl;
        return 0;
      }
    }
    cout << "文件不存在!" << endl;
    return 1;
  }
}

int rm() {
  char filename[FILENAME_LENGTH];
  cin >> filename;
  temp = new filenode;

  if (recent->child) {
    temp = recent->child;
    while (temp->next &&
           (strcmp(temp->filename, filename) != 0 || temp->isdir != 0))
      temp = temp->next;
    if (strcmp(temp->filename, filename) != 0 || temp->isdir != 0) {
      cout << "不存在该文件！" << endl;
      return 0;
    }
  } else {
    cout << "不存在该文件！" << endl;
    return 0;
  }

  if (temp->parent == NULL) {
    temp->prev->next = temp->next;
    if (temp->next)
      temp->next->prev = temp->prev;
    else
      temp->prev->next = NULL;

  } else {
    temp->parent->child = temp->next;
  }
  delete temp;
  cout << "文件已删除!" << endl;
  return 0;
}

int rmdir() {
  char filename[FILENAME_LENGTH];
  cin >> filename;
  temp = new filenode;

  if (recent->child) {
    temp = recent->child;
    while (temp->next &&
           (strcmp(temp->filename, filename) != 0 || temp->isdir != 1))
      temp = temp->next;
    if (strcmp(temp->filename, filename) != 0 || temp->isdir != 1) {
      cout << "不存在该目录！" << endl;
      return 0;
    }
  } else {
    cout << "不存在该目录！" << endl;
    return 0;
  }

  if (temp->child) {
    cout << "目录非空，无法删除！" << endl;
    return 0;
  }

  if (temp->parent == NULL) {
    temp->prev->next = temp->next;
    if (temp->next)
      temp->next->prev = temp->prev;
    else
      temp->prev->next = NULL;

  } else {
    temp->parent->child = temp->next;
  }
  delete temp;
  cout << "目录已删除!" << endl;
  return 0;
}

int cd() {
  char topath[PATH_LENGTH];
  cin >> topath;
  if (strcmp(topath, ".") == 0)
    return 0;
  if (strcmp(topath, "..") == 0) {
    int i;
    while (recent->prev)
      recent = recent->prev; // 向前回溯，找到第一次创建的目录
    if (recent->parent) {
      recent = recent->parent;
    }

    i = strlen(path);
    // printf("%d %s\n",i,path);
    while (path[i] != '/' && i > 0)
      i--; // 找到最右边的/
    if (i != 0) {
      path[i] = '\0';
      // printf("%s",path); //path中不止有一个/
    }

    else
      path[i + 1] = '\0';
  } else {
    findpath(topath);
  }
  return 0;
}

int findpath(char *topath) {
  unsigned int i = 0;
  int sign = 1;
  if (strcmp(topath, "/") == 0) // 如果命令是cd /
  {
    recent = root;
    strcpy(path, "/");
    return 0;
  }
  temp = recent;
  strcpy(temppath, path);
  if (topath[0] == '/') // cd命令以cd /开始
  {
    recent = root->child;
    i++;
    strcpy(path, "/");
    //	printf("\n%s",path);
  } else {
    if (recent != NULL && recent != root) {
      strcat(path, "/");
      //  printf("\n%s\n",path);
    }

    if (recent && recent->child) {
      if (recent->isdir)
        recent = recent->child;
      else {
        printf("路径错误！\n");
        return 1;
      }
    }
  }
  while (i <= strlen(topath) && recent) {
    int j = 0;
    if (topath[i] == '/' && recent->child) {
      i++;
      if (recent->isdir)
        recent = recent->child;
      else {
        printf("路径错误\n");
        return 1;
      }
      strcat(path, "/");
    }
    while (topath[i] != '/' && i <= strlen(topath)) {
      recentpath[j] = topath[i];
      i++;
      j++;
    }
    recentpath[j] = '\0';
    while (
        (strcmp(recent->filename, recentpath) != 0 || (recent->isdir != 1)) &&
        recent->next != NULL) {
      recent = recent->next;
    }
    if (strcmp(recent->filename, recentpath) == 0) {
      if (recent->isdir == 0) {
        strcpy(path, temppath);
        recent = temp;
        printf("是文件不是目录。\n");
        return 1;
      }
      strcat(path, recent->filename);
    }
    if (strcmp(recent->filename, recentpath) != 0 || recent == NULL) {
      strcpy(path, temppath);
      recent = temp;
      printf("输入路径错误\n");
      return 1;
    }
  }
  return 0;
}

int mkdir() {
  temp = initnode(" ", 1);
  cin >> temp->filename;
  if (recent->child == NULL) {
    temp->parent = recent;
    temp->child = NULL;
    recent->child = temp;
    temp->prev = temp->next = NULL;
    printf("目录建立成功!\n");
  } else {
    ttemp = recent->child;
    if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->isdir == 1) {
      {
        printf("目录已存在!\n");
        return 1;
      }
    }
    while (ttemp->next) {
      ttemp = ttemp->next;
      if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->isdir == 1) {
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

int touch() {
  temp = initnode(" ", 0);
  cin >> temp->filename;
  if (recent->child == NULL) {
    temp->parent = recent;
    temp->child = NULL;
    recent->child = temp;
    temp->prev = temp->next = NULL;
    cout << "文件创建成功!" << endl;
  } else {
    ttemp = recent->child;
    if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->isdir == 0) {
      printf("文件已存在!\n");
      return 1;
    }

    while (ttemp->next) {
      ttemp = ttemp->next;
      if (strcmp(ttemp->filename, temp->filename) == 0 && ttemp->isdir == 0) {
        printf("文件已存在!\n");
        return 1;
      }
    }

    ttemp->next = temp;
    temp->parent = NULL;
    temp->child = NULL;
    temp->prev = ttemp;
    temp->next = NULL;
    cout << "文件建立成功!" << endl;
  }
  return 0;
}

int run() {
  cout << "filesystem:" << path << ">";
  cin >> command;
  if (strcmp(command, "mkdir") == 0)
    mkdir();
  else if (strcmp(command, "ls") == 0)
    ls();
  else if (strcmp(command, "cd") == 0)
    cd();
  else if (strcmp(command, "touch") == 0)
    touch();
  else if (strcmp(command, "read") == 0)
    read();
  else if (strcmp(command, "rmdir") == 0)
    rmdir();
  else if (strcmp(command, "write") == 0)
    write();
  else if (strcmp(command, "rm") == 0)
    rm();
  else if (strcmp(command, "help") == 0)
    help();
  else if (strcmp(command, "logout") == 0)
    return 0;
  else
    cout << "请参考help提供的命令列表!" << endl;
  return 1;
}
int main() {
//   cout << "***************************************************************"
//        << endl;
//   cout << "********************操作系统课程设计项目***********************"
//        << endl;
//   cout << "*                     简单文件系统模拟                        *"
//        << endl;
//   cout << "*                   键入help可以获取帮助                      *"
//        << endl;
//   cout << "***************************************************************"
//        << endl;
//   cout << "***************************************************************"
//        << endl;
//   cout << endl;
//   createroot();
//   while (1) {
//     if (!run())
//       break;
//   }
//   return 0;
bool a;
cout<<sizeof(a);
}