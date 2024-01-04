#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#define BUFSIZE 1024

DWORD WINAPI InstanceThread(LPVOID);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD);

int _tmain(VOID)
{
   BOOL fConnected = FALSE;
   DWORD dwThreadId = 0;
   HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
   LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

   // 主循环创建了一个命名管道的实例然后等待一个客户端连接它。
   // 当客户端连接时，一个线程被创建来处理与那个客户端的通信
   // 这个循环可以自由的等待下一个客户端连接请求，这是一个无限循环。

   for (;;)
   {
      _tprintf(TEXT("\nPipe Server: Main thread awaiting client connection on %s\n"), lpszPipename);
      hPipe = CreateNamedPipe(
          lpszPipename,               // pipe name
          PIPE_ACCESS_DUPLEX,         // read/write access
          PIPE_TYPE_MESSAGE |         // message type pipe
              PIPE_READMODE_MESSAGE | // message-read mode
              PIPE_WAIT,              // blocking mode
          PIPE_UNLIMITED_INSTANCES,   // max. instances
          BUFSIZE,                    // output buffer size
          BUFSIZE,                    // input buffer size
          0,                          // client time-out
          NULL);                      // default security attribute

      if (hPipe == INVALID_HANDLE_VALUE)
      {
         _tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
         return -1;
      }

      // 等待客户端连接，如果成功连接
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
      printf("\nERROR - Pipe Server Failure:\n");
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
      printf("\nERROR - Pipe Server Failure:\n");
      printf("   InstanceThread got an unexpected NULL heap allocation.\n");
      printf("   InstanceThread exitting.\n");
      if (pchReply != NULL)
         HeapFree(hHeap, 0, pchReply);
      return (DWORD)-1;
   }

   if (pchReply == NULL)
   {
      printf("\nERROR - Pipe Server Failure:\n");
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
      GetAnswerToRequest(pchRequest, pchReply, &cbReplyBytes);

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

VOID GetAnswerToRequest(LPTSTR pchRequest, LPTSTR pchReply, LPDWORD pchBytes)
{


   // Check the outgoing message to make sure it's not too long for the buffer.
   if (FAILED(StringCchCopy(pchReply, BUFSIZE, TEXT("default answer from server"))))
   {
      *pchBytes = 0;
      pchReply[0] = 0;
      printf("StringCchCopy failed, no outgoing message.\n");
      return;
   }
   *pchBytes = (lstrlen(pchReply) + 1) * sizeof(TCHAR);
}