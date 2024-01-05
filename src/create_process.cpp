#include <cstring>
#include <string.h>
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <tchar.h>
#include <cstdint>
#include <inter_process.h>
using namespace std;
int client(LPTSTR pipe_name, uint8_t *req, DWORD req_size, uint8_t *res, DWORD *res_size);
int main()
{
    std::string commands = GetCommandLineA();
    size_t index1 = commands.find_first_of(' ', 0);
    size_t index2 = commands.find_first_of(' ', index1 + 1);
    string filepath = commands.substr(index1 + 1, index2 - index1 - 1);
    string contain = commands.substr(index2 + 1, commands.size() - index2 - 1);

    uint8_t req[1 + sizeof(FM_CreateFile)]={0};
    req[0] = FM_CREATE_FILE;
    FM_CreateFile *p = (FM_CreateFile *)(req + 1);
    memcpy(p->file_path, filepath.c_str(), filepath.length());
    memcpy(p->file_content, contain.c_str(), contain.length());
    DWORD size = 1;
    uint8_t state;
    client(FILE_MANAGE_PIPE, req, 1 + sizeof(FM_CreateFile), (uint8_t *)&state, &size);
}

int client(LPTSTR pipe_name, uint8_t *req, DWORD req_size, uint8_t *res, DWORD *res_size)
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