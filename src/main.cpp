#include <w32api.h>
#include <cstring>
#include <iostream>
#include <inter_process.h>
using namespace std;
char current_dir[30] = {'/'};
int MakeCreateProcess();
int Run();
int MakeDeleteProcess();
int client(LPTSTR pipe_name, uint8_t *req, DWORD req_size, uint8_t *res, DWORD *res_size, bool ret);
int main()
{
    while (1)
    {
        if (!Run())
        {
            break;
        }
    }
}

int Run()
{
    cout << current_dir << '>';
    char command[30];
    cin >> command;
    if (strcmp(command, "create") == 0)
        MakeCreateProcess();
    else if (strcmp(command, "del") == 0)
    {
        MakeDeleteProcess();
    }
    else if (strcmp(command, "mkdir") == 0)
    {
        uint8_t req[1 + sizeof(FM_CreateDir)] = {FM_CREATE_DIR};
        FM_CreateDir *p = (FM_CreateDir *)(req + 1);
        cin >> p->file_name;
        uint8_t res;
        DWORD size;
        client(FILE_MANAGE_PIPE, req, 1 + sizeof(FM_CreateDir), &res, &size, false);
    }
    else if (strcmp(command, "dir") == 0) ///
    {
        uint8_t req[1] = {FM_SHOW_DIR};
        uint8_t res_data[1024] = {0};
        DWORD res_size;
        client(FILE_MANAGE_PIPE, req, 1, res_data, &res_size, true);
        res_data[res_size] = 0;
        cout << res_data;
    }
    else if (strcmp(command, "cd") == 0)
    {
        uint8_t req[1 + sizeof(FM_ChangeDir)] = {FM_CHANGE_DIR};
        FM_ChangeDir *p = (FM_ChangeDir *)(req + 1);
        cin >> p->file_name;
        DWORD size;
        client(FILE_MANAGE_PIPE, req, 1 + sizeof(FM_ChangeDir), (uint8_t *)(current_dir), &size,true);
        current_dir[size] = 0;
    }
    else if (strcmp(command, "rm") == 0)
    {
        uint8_t req[1 + sizeof(FM_DeleteDir)] = {FM_DELETE_DIR};
        FM_DeleteDir *p = (FM_DeleteDir *)(req + 1);
        cin >> p->file_name;
        uint8_t res;
        DWORD size;
        client(FILE_MANAGE_PIPE, req, 1 + sizeof(FM_DeleteDir), &res, &size,false);
    }
    return 1;
}

int MakeCreateProcess()
{
    char file_path[30];
    char file_content[255];
    cin >> file_path >> file_content;
    STARTUPINFOA stStartUpInfo;
    ::memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
    stStartUpInfo.cb = sizeof(stStartUpInfo);
    PROCESS_INFORMATION stProcessInfo;
    ::memset(&stProcessInfo, 0, sizeof(stProcessInfo));

    int cmd_len = strlen(file_content) + 2 + strlen(file_path);
    char szCmd[cmd_len] = {' '};
    memcpy(szCmd + 1, file_path, strlen(file_path));

    memcpy(szCmd + strlen(file_path) + 1, file_content, strlen(file_content));
    szCmd[strlen(file_path)] = ' ';
    // 执行文件的路径需要修改
    char szPath[] = "D:\\Data\\Code\\C++_Code\\OS\\build\\create_process.exe";
    // char szCmd[] = " http://community.csdn.net/"; // lpCommandLine的内容中开头需要一个空格，不然就和lpApplicationName连在一起去了

    bool bRet = ::CreateProcessA(
        szPath,
        szCmd,
        NULL,
        NULL,
        false,
        0,
        NULL,
        NULL,
        &stStartUpInfo,
        &stProcessInfo);

    if (bRet)
    {
        // 等待3s后关闭进程
        WaitForSingleObject(stProcessInfo.hProcess, 30000L);
        ::CloseHandle(stProcessInfo.hProcess);
        ::CloseHandle(stProcessInfo.hThread);
        stProcessInfo.hProcess = NULL;
        stProcessInfo.hThread = NULL;
        stProcessInfo.dwProcessId = 0;
        stProcessInfo.dwThreadId = 0;
    }
    else
    {
        // 如果创建进程失败，报错
        printf("create process failed");
    }
    return 0;
}

int MakeDeleteProcess()
{
    char file_path[30] = {0};
    cin >> file_path;
    STARTUPINFOA stStartUpInfo;
    ::memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
    stStartUpInfo.cb = sizeof(stStartUpInfo);
    PROCESS_INFORMATION stProcessInfo;
    ::memset(&stProcessInfo, 0, sizeof(stProcessInfo));

    int cmd_len = 1 + strlen(file_path);
    char szCmd[cmd_len] = {' '};
    memcpy(szCmd + 1, file_path, strlen(file_path));

    // 执行文件的路径需要修改
    char szPath[] = "D:\\Data\\Code\\C++_Code\\OS\\build\\delete.exe";
    // char szCmd[] = " http://community.csdn.net/"; // lpCommandLine的内容中开头需要一个空格，不然就和lpApplicationName连在一起去了

    bool bRet = ::CreateProcessA(
        szPath,
        szCmd,
        NULL,
        NULL,
        false,
        0,
        NULL,
        NULL,
        &stStartUpInfo,
        &stProcessInfo);

    if (bRet)
    {
        // 等待3s后关闭进程
        WaitForSingleObject(stProcessInfo.hProcess, 30000L);
        ::CloseHandle(stProcessInfo.hProcess);
        ::CloseHandle(stProcessInfo.hThread);
        stProcessInfo.hProcess = NULL;
        stProcessInfo.hThread = NULL;
        stProcessInfo.dwProcessId = 0;
        stProcessInfo.dwThreadId = 0;
    }
    else
    {
        // 如果创建进程失败，查看错误码
        printf("create process failed");
    }
}

// int MakeReadProcess()
// {
//     char file_path[30];
//     cin >> file_path;
//     STARTUPINFO stStartUpInfo;
//     ::memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
//     stStartUpInfo.cb = sizeof(stStartUpInfo);
//     PROCESS_INFORMATION stProcessInfo;
//     ::memset(&stProcessInfo, 0, sizeof(stProcessInfo));

//     int cmd_len = 1 + strlen(file_path);
//     char szCmd[cmd_len];
//     szCmd[0] = ' ';
//     strcat(szCmd, file_path);

//     // 执行文件的路径需要修改
//     char szPath[] = "c:\\program files\\internet explorer\\iexplore.exe";
//     // char szCmd[] = " http://community.csdn.net/"; // lpCommandLine的内容中开头需要一个空格，不然就和lpApplicationName连在一起去了

//     bool bRet = ::CreateProcess(
//         szPath,
//         szCmd,
//         NULL,
//         NULL,
//         false,
//         CREATE_NEW_CONSOLE,
//         NULL,
//         NULL,
//         &stStartUpInfo,
//         &stProcessInfo);

//     if (bRet)
//     {
//         // 等待3s后关闭进程
//         WaitForSingleObject(stProcessInfo.hProcess, 3000L);
//         ::CloseHandle(stProcessInfo.hProcess);
//         ::CloseHandle(stProcessInfo.hThread);
//         stProcessInfo.hProcess = NULL;
//         stProcessInfo.hThread = NULL;
//         stProcessInfo.dwProcessId = 0;
//         stProcessInfo.dwThreadId = 0;
//     }
//     else
//     {
//         // 如果创建进程失败，查看错误码
//         printf("create process failed");
//     }
// }

// int MakeFileManageProcess(int mode)
// {
//     char dir_name[8];
//     cin >> dir_name;
//     STARTUPINFO stStartUpInfo;
//     ::memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
//     stStartUpInfo.cb = sizeof(stStartUpInfo);
//     PROCESS_INFORMATION stProcessInfo;
//     ::memset(&stProcessInfo, 0, sizeof(stProcessInfo));

//     int cmd_len = 3 + strlen(dir_name);
//     char szCmd[cmd_len];
//     if (mode)
//     {
//         // 生成目录
//         char temp_chars[] = " 4 ";
//         strcpy(szCmd, temp_chars);
//     }
//     else
//     {
//         //删除目录
//         char temp_chars[] = " 5 ";
//         strcpy(szCmd, temp_chars);
//     }
//     strcat(szCmd, dir_name);

//     // 执行文件的路径需要修改
//     char szPath[] = "c:\\program files\\internet explorer\\iexplore.exe";
//     // char szCmd[] = " http://community.csdn.net/"; // lpCommandLine的内容中开头需要一个空格，不然就和lpApplicationName连在一起去了

//     bool bRet = ::CreateProcess(
//         szPath,
//         szCmd,
//         NULL,
//         NULL,
//         false,
//         CREATE_NEW_CONSOLE,
//         NULL,
//         NULL,
//         &stStartUpInfo,
//         &stProcessInfo);

//     if (bRet)
//     {
//         // 等待3s后关闭进程
//         WaitForSingleObject(stProcessInfo.hProcess, 3000L);
//         ::CloseHandle(stProcessInfo.hProcess);
//         ::CloseHandle(stProcessInfo.hThread);
//         stProcessInfo.hProcess = NULL;
//         stProcessInfo.hThread = NULL;
//         stProcessInfo.dwProcessId = 0;
//         stProcessInfo.dwThreadId = 0;
//     }
//     else
//     {
//         // 如果创建进程失败，查看错误码
//         printf("create process failed");
//     }
// }

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