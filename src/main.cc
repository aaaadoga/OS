#include <w32api.h>
#include <cstring>
#include <iostream>
#include <windows.h>
using namespace std;

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
    char command[30];
    cin >> command;
    if (strcmp(command, "create") == 0)
        MakeCreateProcess();
    else if (strcmp(command, "del") == 0)
        MakeDeleteProcess();
    else if (strcmp(command, "read") == 0)
        MakeReadProcess();
    else if (strcmp(command, "mkdir") == 0)
        MakeFileManageProcess(1);
    else if (strcmp(command, "rm") == 0)
        MakeFileManageProcess(0);
    return 1;
}

int MakeCreateProcess()
{
    char file_path[30];
    char file_content[255];
    cin >> file_path >> file_content;
    STARTUPINFO stStartUpInfo;
    ::memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
    stStartUpInfo.cb = sizeof(stStartUpInfo);
    PROCESS_INFORMATION stProcessInfo;
    ::memset(&stProcessInfo, 0, sizeof(stProcessInfo));

    int cmd_len = strlen(file_content) + 2 + strlen(file_path);
    char szCmd[cmd_len];
    szCmd[0] = ' ';
    strcat(szCmd, file_path);
    int temp_len = strlen(szCmd);
    szCmd[temp_len] = ' ';
    strcat(szCmd, file_content);

    // 执行文件的路径需要修改
    char szPath[] = "c:\\program files\\internet explorer\\iexplore.exe";
    // char szCmd[] = " http://community.csdn.net/"; // lpCommandLine的内容中开头需要一个空格，不然就和lpApplicationName连在一起去了

    bool bRet = ::CreateProcess(
        szPath,
        szCmd,
        NULL,
        NULL,
        false,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &stStartUpInfo,
        &stProcessInfo);

    if (bRet)
    {
        // 等待3s后关闭进程
        WaitForSingleObject(stProcessInfo.hProcess, 3000L);
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

int MakeDeleteProcess()
{
    char file_path[30];
    cin >> file_path;
    STARTUPINFO stStartUpInfo;
    ::memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
    stStartUpInfo.cb = sizeof(stStartUpInfo);
    PROCESS_INFORMATION stProcessInfo;
    ::memset(&stProcessInfo, 0, sizeof(stProcessInfo));

    int cmd_len = 1 + strlen(file_path);
    char szCmd[cmd_len];
    szCmd[0] = ' ';
    strcat(szCmd, file_path);

    // 执行文件的路径需要修改
    char szPath[] = "c:\\program files\\internet explorer\\iexplore.exe";
    // char szCmd[] = " http://community.csdn.net/"; // lpCommandLine的内容中开头需要一个空格，不然就和lpApplicationName连在一起去了

    bool bRet = ::CreateProcess(
        szPath,
        szCmd,
        NULL,
        NULL,
        false,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &stStartUpInfo,
        &stProcessInfo);

    if (bRet)
    {
        // 等待3s后关闭进程
        WaitForSingleObject(stProcessInfo.hProcess, 3000L);
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

int MakeReadProcess()
{
    char file_path[30];
    cin >> file_path;
    STARTUPINFO stStartUpInfo;
    ::memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
    stStartUpInfo.cb = sizeof(stStartUpInfo);
    PROCESS_INFORMATION stProcessInfo;
    ::memset(&stProcessInfo, 0, sizeof(stProcessInfo));

    int cmd_len = 1 + strlen(file_path);
    char szCmd[cmd_len];
    szCmd[0] = ' ';
    strcat(szCmd, file_path);

    // 执行文件的路径需要修改
    char szPath[] = "c:\\program files\\internet explorer\\iexplore.exe";
    // char szCmd[] = " http://community.csdn.net/"; // lpCommandLine的内容中开头需要一个空格，不然就和lpApplicationName连在一起去了

    bool bRet = ::CreateProcess(
        szPath,
        szCmd,
        NULL,
        NULL,
        false,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &stStartUpInfo,
        &stProcessInfo);

    if (bRet)
    {
        // 等待3s后关闭进程
        WaitForSingleObject(stProcessInfo.hProcess, 3000L);
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

int MakeFileManageProcess(int mode)
{
    char dir_name[8];
    cin >> dir_name;
    STARTUPINFO stStartUpInfo;
    ::memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
    stStartUpInfo.cb = sizeof(stStartUpInfo);
    PROCESS_INFORMATION stProcessInfo;
    ::memset(&stProcessInfo, 0, sizeof(stProcessInfo));

    int cmd_len = 3 + strlen(dir_name);
    char szCmd[cmd_len];
    if (mode)
    {
        // 生成目录
        char temp_chars[] = " 4 ";
        strcpy(szCmd, temp_chars);
    }
    else
    {
        //删除目录
        char temp_chars[] = " 5 ";
        strcpy(szCmd, temp_chars);
    }
    strcat(szCmd, dir_name);

    // 执行文件的路径需要修改
    char szPath[] = "c:\\program files\\internet explorer\\iexplore.exe";
    // char szCmd[] = " http://community.csdn.net/"; // lpCommandLine的内容中开头需要一个空格，不然就和lpApplicationName连在一起去了

    bool bRet = ::CreateProcess(
        szPath,
        szCmd,
        NULL,
        NULL,
        false,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &stStartUpInfo,
        &stProcessInfo);

    if (bRet)
    {
        // 等待3s后关闭进程
        WaitForSingleObject(stProcessInfo.hProcess, 3000L);
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