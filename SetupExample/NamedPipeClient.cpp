#include "NamedPipeClient.h"
#include "SafeFunc.h"
#include <corecrt_wstdio.h>
#include "logging.h"

#define MODULE_TAG  ("NamedPipeClient")

bool NamedPipeClient::send_request(bool to_service, uint8_t* req, int req_len, CommmonPipePacket* resp)
{
    BOOL fSuccess;
    DWORD cbRead;
    int  pipe_index = 1;

    if (!pipeName_valid)
    {
        TCHAR pipe_name[MAX_PATH];
        SecureZeroMemory(pipe_name, sizeof(pipe_name));
        SafeFunc::Instance()->ReadServicePipeName(pipe_name, sizeof(pipe_name) / 2);

        swprintf_s(pipeName[0], MAX_PATH, TEXT("\\\\.\\pipe\\%s22"), pipe_name);    //to service 
        swprintf_s(pipeName[1], MAX_PATH, TEXT("\\\\.\\pipe\\%s"), pipe_name);      //to avatar

        pipeName_valid = true;
    }

    if (to_service)
    {
        pipe_index = 0;
    }
    //打开管道 写入数据 读取响应 关闭句柄
    fSuccess = CallNamedPipe(
        pipeName[pipe_index],       // pipe name 
        (LPVOID)req,    // message to server 
        req_len,
        resp,           // buffer to receive reply 
        sizeof(*resp),  // size of read buffer 
        &cbRead,        // number of bytes read 
        5000);         // waits for 5 seconds 

    if (!fSuccess)
    {
        auto e = GetLastError();
        if (e == ERROR_MORE_DATA)
        {
            Print_Debug("More data, read %u bytes\n", cbRead);
        }
          
        //else
        //  Print_Error("CallNamedPipe error %u\n", e);
    }

    if (pipe_index == 0)
    {
        char tmpString[MAX_PATH];

        WideCharToMultiByte(CP_ACP, 0, pipeName[pipe_index], -1, tmpString, sizeof(tmpString) - 1, NULL, FALSE);

        Print_Debug("send message from %s  %d pipe %s\n", tmpString, pipe_index, (fSuccess) ? "success" : "failed");
    }

    return fSuccess;
}
