#include <cstring>
#include <disk_manage.h>
#include <iostream>
DWORD WINAPI InstanceThread(LPVOID lpvParam);
int client(LPTSTR pipe_name, uint8_t *req, DWORD req_size, uint8_t *res, DWORD *res_size,bool ret);

VOID GetAnswerToRequest(uint8_t *pchRequest, DWORD cbBytesRead,
                        uint8_t *pchReply,
                        LPDWORD pchBytes);

uint64_t _BITMAP[16] = {0};
FileBlock fileBlocks[1024];
std::fstream file("disk.dat", std::ios::in | std::ios::out | std::ios::binary);

int main()
{
  // 判断文件是否存在
  if (!file)
  {
    std::cout << "file not exist" << std::endl;
    // 创建文件
    file.open("disk.dat", std::ios::out | std::ios::binary);
    // 格式化磁盘
    std::cout << "formating" << std::endl;
    format();
    file.close();
    file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
  }
  // 读取磁盘,检查磁盘完整性
  file.seekg(0, std::ios::beg);
  inode root;
  file.read(reinterpret_cast<char *>(&root), sizeof(FileBlock));
  file.seekg(1024 * sizeof(FileBlock), std::ios::beg);
  file.read(reinterpret_cast<char *>(&_BITMAP), sizeof(uint64_t));

  if ((_BITMAP[0] & 0x8000000000000000) == 0 || (root.mode & 0x8000) == 0)
  {
    std::cout << "disk error,formating" << std::endl;
    format();
    file.close();
    file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
  }
  // 检查完毕
  std::cout << "disk ok" << std::endl;
  ///////////////////////////////////////////

  // uint16_t addr;
  // char t[]="dajiahao yao ,woshi shuode daoli ";
  // createFile(0,"hello",&addr,t,40);

  ////////////////////////////////////////////////////////////////////////////////////////////
  // 开启服务器

  BOOL fConnected = FALSE;
  DWORD dwThreadId = 0;
  HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
  LPCTSTR lpszPipename = DISK_MANAGE_PIPE;

  // 主循环创建了一个命名管道的实例然后等待一个客户端连接它。
  // 当客户端连接时，一个线程被创建来处理与那个客户端的通信
  // 这个循环可以自由的等待下一个客户端连接请求，这是一个无限循环。

  for (;;)
  {
    _tprintf(TEXT("\nDisk Manage Server: Main thread awaiting client connection on %s\n"), lpszPipename);
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

int format()
{
  // 初始化位图
  memset(_BITMAP, 0, 16 * sizeof(uint64_t));
  // 初始化Block
  memset(fileBlocks, 0, 1024 * sizeof(FileBlock));

  // 初始化根目录
  inode *root = reinterpret_cast<inode *>(&fileBlocks[0]);
  root->mode = 0x8000;

  // 设置位图
  _BITMAP[0] |= 0x8000000000000000;
  // 写入磁盘

  file.seekp(0, std::ios::beg);

  file.write(reinterpret_cast<char *>(fileBlocks), 1024 * sizeof(FileBlock));
  file.write(reinterpret_cast<char *>(_BITMAP), 16 * sizeof(uint64_t));
  // 刷新缓冲区
  return 0;
}

// addr为地址数组，长度为blockNum
int getFreeBlock(uint16_t *addr, uint16_t blockNum)
{
  char blank[40] = {0};
  // 读取位图
  file.seekg(1024 * sizeof(FileBlock), std::ios::beg);
  file.read(reinterpret_cast<char *>(_BITMAP), 16 * sizeof(uint64_t));
  // 遍历位图,找到空闲块并设置位图
  for (int i = 0; i < 16; i++)
  {
    if (_BITMAP[i] == 0xffffffffffffffff)
    {
      continue;
    }
    for (int j = 0; blockNum && j < 64; j++)
    {
      if (i * 64 + j == 900)
        return -1;
      if ((_BITMAP[i] & (0x8000000000000000 >> j)) == 0)
      {
        _BITMAP[i] |= (0x8000000000000000 >> j);
        addr[blockNum - 1] = i * 64 + j;
        file.seekp((i * 64 + j) * sizeof(FileBlock), std::ios::beg);
        file.write(blank, 40);
        blockNum--;
      }
    }
  }
  // 写入位图
  if (blockNum == 0)
  {
    file.seekp(1024 * sizeof(FileBlock), std::ios::beg);
    file.write(reinterpret_cast<char *>(_BITMAP), 16 * sizeof(uint64_t));
    file.close();
    file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
    return 0;
  }
  // 空间不足
  return -1;
}

int deleteBlock(uint16_t *addr, uint16_t blockNum)
{

  // 读取位图
  file.seekg(1024 * sizeof(FileBlock), std::ios::beg);
  file.read(reinterpret_cast<char *>(_BITMAP), 16 * sizeof(uint64_t));

  // 遍历位图,找到非空闲块并设置位图
  for (int i = 0; i < blockNum; i++)
  {
    _BITMAP[addr[i] / 64] &= ~(0x8000000000000000 >> (addr[i] % 64));
  }
  // 写入位图
  file.seekp(1024 * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(_BITMAP), 16 * sizeof(uint64_t));
  file.close();
  file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
  return 0;
}

int deleteInode(uint16_t dir_addr, uint16_t addr)
{
  if (addr == 0)
  { // 根目录不可删除
    return -1;
  }
  // 读取父目录
  file.seekg(dir_addr * sizeof(FileBlock), std::ios::beg);
  inode dir;
  file.read(reinterpret_cast<char *>(&dir), sizeof(FileBlock));

  // 查找父目录内容，删除目录项
  DirBlock dir_block;
  file.seekg(dir.addr[(dir.size - 1) / 4] * sizeof(FileBlock),
             std::ios::beg); // 最后一个目录块
  file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));

  dir_item temp = dir_block.data[dir.size % 4 - 1]; // 最后一个目录项

  for (int i = 0; i < dir.size % 4; i++)
  { // 最后一个目录块
    if (dir_block.data[i].addr == addr)
    {
      dir_block.data[i] = temp; // 覆盖
      dir.size--;
      // 写入父目录块文件
      file.seekp(dir.addr[(dir.size - 1) / 4] * sizeof(FileBlock),
                 std::ios::beg);
      file.write(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));
      break;
    }
  }

  for (int i = 0; i < (dir.size - 1) / 4; i++)
  { // 其他目录块
    // 读取目录块
    file.seekg(dir.addr[i] * sizeof(FileBlock), std::ios::beg);
    file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));

    for (int j = 0; j < 4; j++)
    { // 遍历目录项
      if (dir_block.data[j].addr == addr)
      {
        dir_block.data[j] = temp; // 覆盖
        dir.size--;
        // 写入父目录块文件
        file.seekp(dir.addr[i] * sizeof(FileBlock), std::ios::beg);
        file.write(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));
        break;
      }
    }
  }
  // 写入父目录inode
  file.seekp(dir_addr * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir), sizeof(FileBlock));

  // 文件删除，读入文件inode
  file.seekg(addr * sizeof(FileBlock), std::ios::beg);
  inode file_node;
  file.read(reinterpret_cast<char *>(&file_node), sizeof(FileBlock));

  if (file_node.size)
  // 将文件块装入addr_list
  {
    uint16_t addr_list[(file_node.size - 1) / 40 + 1 + 1];
    memcpy(addr_list, file_node.addr,
           sizeof(uint16_t) * ((file_node.size - 1) / 40 + 1));
    addr_list[(file_node.size - 1) / 40 + 1] = addr;
    // 删除文件块
    deleteBlock(addr_list, (file_node.size - 1) / 40 + 1 + 1);
  }
  else
  {
    deleteBlock(&addr, 1);
  }

  return 0;
}

int createFile(uint16_t dir_addr, const char *name, uint16_t *file_addr,
               char *buffer, int size)
{
  // dir_addr为父目录地址，name为文件名，file_addr为文件地址，buffer为文件内容，size为文件大小

  // 申请空闲块
  uint16_t addr[(size - 1) / 40 + 1 + 1];
  getFreeBlock(addr, (size - 1) / 40 + 1 + 1);
  *file_addr = addr[0]; // 文件inode地址

  // 写入文件inode块
  inode file_node;
  file_node.mode = 0x0000;
  file_node.size = size;
  memcpy(file_node.addr, addr + 1, sizeof(uint16_t) * ((size - 1) / 40 + 1));
  file.seekp(addr[0] * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&file_node), sizeof(FileBlock));

  for (int i = 1; i < (size - 1) / 40 + 1 + 1; i++)
  { // 写入文件内容
    file.seekp(addr[i] * sizeof(FileBlock), std::ios::beg);
    file.write(buffer + (i - 1) * 40, sizeof(FileBlock));
  }

  // 读取父目录，写入目录项
  file.seekg(dir_addr * sizeof(FileBlock), std::ios::beg);
  inode dir;
  file.read(reinterpret_cast<char *>(&dir), sizeof(FileBlock));
  DirBlock dir_block;
  // 写入目录项
  if (dir.size % 4 == 0)
  {
    getFreeBlock(&dir.addr[dir.size / 4], 1);
  }
  else
  {
    file.seekg(dir.addr[dir.size / 4] * sizeof(FileBlock), std::ios::beg);
    file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));
  }
  dir_block.data[dir.size % 4].addr = addr[0];
  memcpy(dir_block.data[dir.size % 4].name, name, 8);
  file.seekp(dir.addr[dir.size / 4] * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));
  // 写入父目录inode
  dir.size++;
  file.seekp(dir_addr * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir), sizeof(FileBlock));
  file.close();
  file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
  return 0;
}

int createDir(uint16_t dir_addr, char *name, uint16_t *child_dir_addr)
{

  // 申请空闲块
  uint16_t addr;
  getFreeBlock(&addr, 1);
  *child_dir_addr = addr; // 子目录inode地址
  std::cout<<addr;

  // 写入子目录inode块
  inode dir_node;
  dir_node.mode = 0x8000;
  dir_node.size = 0;
  file.seekp(addr * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir_node), sizeof(FileBlock));

  // 读取父目录，写入目录项
  file.seekg(dir_addr * sizeof(FileBlock), std::ios::beg);
  inode dir;
  file.read(reinterpret_cast<char *>(&dir), sizeof(FileBlock));
  DirBlock dir_block;
  // 写入目录项
  if (dir.size % 4 == 0)
  {
    getFreeBlock(&dir.addr[dir.size / 4], 1);
  }
  else
  {
    file.seekg(dir.addr[dir.size / 4] * sizeof(FileBlock), std::ios::beg);
    file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));
  }
  dir_block.data[dir.size % 4].addr = addr;
  memcpy(dir_block.data[dir.size % 4].name, name, 8);
  file.seekp(dir.addr[dir.size / 4] * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));

  dir.size++;
  file.seekp(dir_addr * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir), sizeof(FileBlock));
  file.close();
  file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
  return 0;
}

int loadFile(uint16_t addr, char *buffer)
{
  // 读取文件inode
  file.seekg(addr * sizeof(FileBlock), std::ios::beg);
  inode file_node;
  file.read(reinterpret_cast<char *>(&file_node), sizeof(FileBlock));

  for (int i = 0; i < (file_node.size - 1) / 40 + 1; i++)
  { // 读取文件内容
    file.seekg(file_node.addr[i] * sizeof(FileBlock), std::ios::beg);
    file.read(buffer + i * 40, sizeof(FileBlock));
  }

  return 0;
}

int readDir(uint16_t addr, FCB_P *buffer, uint16_t *size)
{
  // 读取目录inode
  file.seekg(addr * sizeof(FileBlock), std::ios::beg);
  inode dir;
  file.read(reinterpret_cast<char *>(&dir), sizeof(FileBlock));
  *size = dir.size;
  uint8_t pointer = 0;

  // 查找目录内容
  DirBlock dir_block;
  file.seekg(dir.addr[(dir.size - 1) / 4] * sizeof(FileBlock),
             std::ios::beg); // 最后一个目录块
  file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));

  inode child_inode;
  for (int i = 0; i < dir.size % 4; i++)
  { // 最后一个目录块
    buffer[pointer].addr = dir_block.data[i].addr;
    memcpy(buffer[pointer].name, dir_block.data[i].name, 8);
    file.seekg(dir_block.data[i].addr * sizeof(FileBlock), std::ios::beg);
    file.read(reinterpret_cast<char *>(&child_inode), sizeof(FileBlock));
    buffer[pointer].mode = child_inode.mode;
    buffer[pointer].size = child_inode.size;
    pointer++;
  }

  for (int i = 0; i < (dir.size - 1) / 4; i++)
  { // 其他目录块
    // 读取目录块
    file.seekg(dir.addr[i] * sizeof(FileBlock), std::ios::beg);
    file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));

    for (int j = 0; j < 4; j++)
    { // 遍历目录项
      buffer[pointer].addr = dir_block.data[j].addr;
      memcpy(buffer[pointer].name, dir_block.data[j].name, 8);
      file.seekg(dir_block.data[j].addr * sizeof(FileBlock), std::ios::beg);
      file.read(reinterpret_cast<char *>(&child_inode), sizeof(FileBlock));
      buffer[pointer].mode = child_inode.mode;
      buffer[pointer].size = child_inode.size;
      pointer++;
    }
  }
  return 0;
}

int readBlock(uint16_t addr, FileBlock *buffer)
{
  file.seekg((addr) * sizeof(FileBlock), std::ios::beg);
  file.read(reinterpret_cast<char *>(buffer), sizeof(FileBlock));
  return 0;
}

int loadToMemory(uint8_t index, uint16_t inode_addr)
{
  uint8_t req[1 + sizeof(MM_load_file)] = {0};
  req[0] = MM_LOADFILE;
  MM_load_file *p = (MM_load_file *)(req + 1);
  p->index = index;
  file.seekg((inode_addr) * sizeof(FileBlock), std::ios::beg);
  inode file_node;
  file.read(reinterpret_cast<char *>(&file_node), sizeof(FileBlock));
  p->size = (file_node.size - 1) / 4 + 1;
  memcpy(p->inode_addr, file_node.addr, p->size);
  DWORD size;
  client(MEMORY_MANAGE_PIPE, req, 1 + sizeof(MM_load_file), req, &size,false);
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

    // Write the reply to the pipe.
    if (cbReplyBytes == 0)
    {
      break;
    }

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
  // show();
  return 1;
}

VOID GetAnswerToRequest(uint8_t *pchRequest, DWORD cbBytesRead,
                        uint8_t *pchReply,
                        LPDWORD pchBytes)
{
  uint8_t *type = (uint8_t *)(pchRequest);
  if (*type == DM_FORMAT)
  {
    format();
  }
  else if (*type == DM_CREATEFILE) // 创建文件
  {
    DM_createFile *p = (DM_createFile *)(pchRequest + 1); // 参数
    createFile(p->dir_addr, p->name, (uint16_t *)pchReply, p->buffer, p->size);
    std::cout << "createFile success\n";
    *pchBytes = 2;
  }
  else if (*type == DM_CREATEDIR) // 创建目录
  {
    DM_createDir *p = (DM_createDir *)(pchRequest + 1);
    createDir(p->dir_addr, p->name, (uint16_t *)(pchReply));
    std::cout << "createDir success\n";
    *pchBytes = 2;
  }
  else if (*type == DM_DELETEINODE) // 删除目录或文件
  {
    DM_deleteInode *p = (DM_deleteInode *)(pchRequest + 1);
    deleteInode(p->dir_addr, p->addr);
    std::cout << "deleteInode success\n";
    *pchBytes = 0;
  }
  else if (*type == DM_LOADFILE) // 读文件内容
  {
    DM_loadFile *p = (DM_loadFile *)(pchRequest + 1);
    loadFile(p->addr, (char *)pchReply);
    std::cout << "loadFile success\n";
    *pchBytes = 720;
  }
  else if (*type == DM_READDIR) // 读目录
  {
    DM_readDir *p = (DM_readDir *)(pchRequest + 1);
    uint16_t *size = (uint16_t *)(pchReply);         // 多少个目录项
    readDir(p->addr, (FCB_P *)(pchReply + 2), size); //
    std::cout << "readDir success\n";
    *pchBytes = 2 + (*size) * sizeof(FCB_P);
  }
  else if (*type == DM_READBLOCK) // 内存用
  {
    DM_readBlock *p = (DM_readBlock *)(pchRequest + 1);
    readBlock(p->addr, (FileBlock *)(pchReply));
    std::cout << "readBlock success\n";
    *pchBytes = 40;
  }
  else if (*type == DM_LOADTOMEMORY) // 载入内存
  {
    DM_loadToMemory *p = (DM_loadToMemory *)(pchRequest + 1);
    loadToMemory(p->index, p->inode_addr);
    std::cout << "loadToMemory success\n";
    *pchBytes = 0;
  }
  else
  {
    *pchBytes = 0;
  }
}

int client(LPTSTR pipe_name, uint8_t *req, DWORD req_size, uint8_t *res, DWORD *res_size,bool ret)
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
  if(!ret){
    return 0;
  }

  do
  {
    // Read from the pipe.
    fSuccess = ReadFile(
        hPipe,                   // pipe handle
        res,                     // buffer to receive reply
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

int show()
{
  std::cout << "courrent bitmap:\n";
  for (int a = 0; a < 32; a++)
  {
    for (int b = 0; b < 32; b++)
    {
      int index = a * 32 + b;
      int i = index / 64, j = index % 64;
      if ((_BITMAP[i] & (0x8000000000000000 >> j)) == 0)
      {
        std::cout << "0";
      }
      else
      {
        std::cout << "1";
      }
    }
    std::cout << "\n";
  }
  std::cout << "\n";
  return 0;
}
