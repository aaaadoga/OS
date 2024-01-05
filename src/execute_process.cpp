#include <cstring>
#include <iostream>
#include <inter_process.h>
#include <sstream>
#include <vector>
using namespace std;
int client(LPTSTR pipe_name, uint8_t *req, DWORD req_size, uint8_t *res, DWORD *res_size,bool ret);

int main()
{
    std::string commands = GetCommandLineA();
    std::stringstream cmd(commands);
    string path;
    cmd >> path;
    cmd >> path;
    vector<uint8_t> page_list;
    uint8_t num;
    while (cmd >> num)
    {
        page_list.push_back(num);
    }
    // 获取内存块
    uint8_t mem_req[1] = {MM_MALLOCMEMORY};
    uint8_t index;
    DWORD size;
    client(MEMORY_MANAGE_PIPE, mem_req, 1, &index, &size,true);

    // 请求文件管理
    uint8_t file_req[1 + sizeof(FM_ReadFile)] = {FM_READ_FILE};
    uint8_t res;
    FM_ReadFile *p1 = (FM_ReadFile *)(file_req + 1);
    memcpy(p1->file_path, path.c_str(), path.length());
    p1->memory_block_NO = index;
    client(FILE_MANAGE_PIPE, file_req, 1 + sizeof(FM_ReadFile), &res, &size,false);

    // 读取页内容
    char buff[40];
    uint8_t page_req[1 + sizeof(MM_request_page)] = {MM_REQUESTPAGE};
    MM_request_page *p2 = (MM_request_page *)(page_req + 1);
    p2->index = index;
    for (auto i : page_list)
    {
        p2->page_id = i;
        client(MEMORY_MANAGE_PIPE, page_req, 1 + sizeof(MM_request_page), (uint8_t *)buff, &size,true);
        cout<<"page:"<<i<<endl;
        printf("%s\n",buff);
    }
    return 0;
};

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
    if(! ret){
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