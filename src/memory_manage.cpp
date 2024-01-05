#include <cstring>
#include <memory_manage.h>
#include <iostream>
DWORD WINAPI InstanceThread(LPVOID lpvParam);

VOID GetAnswerToRequest(uint8_t *pchRequest, DWORD cbBytesRead,
                        uint8_t *pchReply,
                        LPDWORD pchBytes);

MemoryBlock memory[64];

uint8_t _BITMAP;

PageTable page_table[8];

int main()
{

  ////////////////////////////////////////////////////////////////////////////////////////////
  // 开启服务器

  BOOL fConnected = FALSE;
  DWORD dwThreadId = 0;
  HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
  LPCTSTR lpszPipename = MEMORY_MANAGE_PIPE;

  // 主循环创建了一个命名管道的实例然后等待一个客户端连接它。
  // 当客户端连接时，一个线程被创建来处理与那个客户端的通信
  // 这个循环可以自由的等待下一个客户端连接请求，这是一个无限循环。

  for (;;)
  {
    _tprintf(TEXT("\nMemory Manage Server: Main thread awaiting client connection on %s\n"), lpszPipename);
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

int malloc_memory(uint8_t *index)
{

  for (int i = 0; i < 8; i++)
  { // 找到第一个空闲的大内存块
    if ((_BITMAP & (1 << i)) == 0)
    {
      _BITMAP |= (1 << i);
      *index = i; // 返回大内存块号
      return 0;
    }
  }
  return -1; // 内存已满
}

int free_memory(uint8_t index)
{
  _BITMAP &= ~(1 << index);
  return 0;
}

int request_page(uint8_t index, uint8_t page_id, MemoryBlock *data)
{
  if (index >= 8 || page_id >= page_table[index].size)
  { // 越界
    return -1;
  }

  if (page_table[index].is_valid[page_id])
  { // 页面在内存中
    *data = memory[page_table[index].page_addr[page_id]];
    return 0;
  }
  else
  { // 页面不在内存中,从磁盘中读取
    uint8_t page_index = 0;
    if (page_table[index].is_full)
    { // 内存已满
      // 选一页换出，找到最先进入内存的页
      for (int i = 0; i < 18; i++)
      {
        if (page_table[index].is_valid[i] &&
            page_table[index].page_addr[i] ==
                index * 8 + page_table[index].point)
        {
          page_index = i;
          break;
        }
      }
      // 是否被修改
      if (page_table[index].is_dirty[page_index])
      {
        // 写回磁盘
        /// 待补充
      }
      page_table[index].is_valid[page_index] = false; // 不在内存中
    }
    else
    { // 内存未满
      if (page_table[index].point == 7)
      {
        page_table[index].is_full = true;
      }
    }
    // 从磁盘中读取
    MemoryBlock tmp;
    read_from_disk(page_table[index].inode_addr[page_id], &tmp);
    // 读入内存
    memory[index * 8 + page_table[index].point] = tmp;
    *data = tmp;
    // 更新页表
    page_table[index].is_valid[page_id] = true;  // 在内存中
    page_table[index].is_dirty[page_id] = false; // 未修改
    page_table[index].page_addr[page_id] =
        index * 8 + page_table[index].point; // 块号
    page_table[index].point = (page_table[index].point + 1) %
                              8; // 内存满时最先进入内存的页，未满时下一个空闲块

    return 0;
  }
}

int load_file(uint8_t index, uint8_t size, uint16_t *inode_addr)
{
  if (index >= 8 || size > 18)
  { // 错误
    return -1;
  }
  page_table[index].size = size;
  page_table[index].is_full = false;
  page_table[index].point = 0;
  for (int i = 0; i < size; i++)
  {
    page_table[index].inode_addr[i] = inode_addr[i];
    page_table[index].is_valid[i] = false;
    page_table[index].is_dirty[i] = false;
  }
  return 0;
}

int read_from_disk(uint16_t inode_addr, MemoryBlock *memory_block)
{
  uint8_t req[1 + sizeof(DM_readBlock)] = {0};
  req[0] = DM_READBLOCK;
  DM_readBlock *p = (DM_readBlock *)(req + 1);
  p->addr = inode_addr;
  DWORD size;
  client(DISK_MANAGE_PIPE, req, 1 + sizeof(DM_readBlock), (uint8_t *)memory_block, &size, false);
  // std::cout<<size;
  // memcpy(memory_block,temp,40);
  return 0;
}

int client(LPTSTR pipe_name, uint8_t *req, DWORD req_size, uint8_t *res, DWORD *res_size, bool ret)
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
    printf("\nERROR - Memory Manager Server Failure:\n");
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
    printf("\nERROR - Memory Manager Server Failure:\n");
    printf("   InstanceThread got an unexpected NULL heap allocation.\n");
    printf("   InstanceThread exitting.\n");
    if (pchReply != NULL)
      HeapFree(hHeap, 0, pchReply);
    return (DWORD)-1;
  }

  if (pchReply == NULL)
  {
    printf("\nERROR - Memory Manager Server Failure:\n");
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
  show();
  return 1;
}

VOID GetAnswerToRequest(uint8_t *pchRequest, DWORD cbBytesRead,
                        uint8_t *pchReply,
                        LPDWORD pchBytes)
{
  uint8_t *type = (uint8_t *)(pchRequest);
  if (*type == MM_MALLOCMEMORY)
  {
    malloc_memory(pchReply);
    *pchBytes = 1;
  }
  else if (*type == MM_FREEMEMORY)
  {
    MM_free_memory *p = (MM_free_memory *)(pchRequest + 1);
    free_memory(p->index);
    *pchBytes = 0;
  }
  else if (*type == MM_REQUESTPAGE)
  {
    MM_request_page *p = (MM_request_page *)(pchRequest + 1);
    request_page(p->index, p->page_id, (MemoryBlock *)pchReply);
    *pchBytes = 40;
  }
  else if (*type == MM_LOADFILE)
  {
    MM_load_file *p = (MM_load_file *)(pchRequest + 1);
    load_file(p->index, p->size, p->inode_addr);
    *pchBytes = 0;
  }
  else
  {
    *pchBytes = 0;
  }
}

int show()
{
  std::cout << "---------MEM----------" << std::endl;
  for (int i = 0; i < 8; i++)
  { // 找到第一个空闲的大内存块
    if ((_BITMAP & (1 << i)) == 0)
    {
      for (int j = 0; j < 8; j++)
      {
        std::cout << "free\t";
        if (j == 3)
        {
          std::cout << "\n";
        }
      }
    }
    else
    {
      int num;
      int point = 0;
      for (int j = 0; j < 18; j++)
      {
        if (page_table[i].is_valid[j])
          num++;
      }
      for (int j = 0; i < num; j++, point++)
      {
        std::cout << "mall\n";
        if (point == 3)
        {
          std::cout << "\n";
        }
      }
      for (int j = 0; i < 8 - num; j++, point++)
      {
        std::cout << "free\n";
        if (point == 3)
        {
          std::cout << "\n";
        }
      }
    }
    std::cout << "\n----------------------" << std::endl;
  }
}