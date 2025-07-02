#include "NamedPipeClient.h"
#include "SafeFunc.h"
#include <corecrt_wstdio.h>
#include "logging.h"

#define MODULE_TAG  ("NamedPipeClient")

bool NamedPipeClient::send_request_notify_bool()
{
    CommmonPipePacket request, response;

    request.hdr.magic = PIPE_HEADER_MAGIC;
    request.hdr.type = msg_type_notify_multi_boot_req;

    if (send_request(false, (uint8_t*)&request, sizeof(request), &response))
    {
        Print_Debug("pipe client notify multi boot success!\n");
        return true;
    }

    return false;

}

bool NamedPipeClient::send_request_get_shared_memory_handle(HANDLE& shared_memory_handle)
{
    CommmonPipePacket request, response;
    request.hdr.magic = PIPE_HEADER_MAGIC;
    request.hdr.type = msg_type_get_shared_memory_handle_req;
    request.u.req_shared_handle.process_ID = GetCurrentProcessId();

    if (send_request(false, (uint8_t*)&request, sizeof(request), &response))
    {
        shared_memory_handle = (HANDLE)response.u.resp_shared_handle.handle;
        Print_Debug("pipe client get shared memory handle success, memory handle = 0x%x!\n", shared_memory_handle);

        pHookinfo = (hook_info*)MapViewOfFile(
			shared_memory_handle,
			FILE_MAP_READ,
			0, 0,
			sizeof(hook_info));
		if (!pHookinfo)
		{
            Print_Debug("MapViewOfFile failed: %lu\n", GetLastError());
			return false;
		}
        Print_Debug("hook_ver_major: %u\n", pHookinfo->hook_ver_major);
        Print_Debug("hook_ver_minor: %u\n", pHookinfo->hook_ver_minor);
        Print_Debug("Mouse: %.2f, %.2f\n", pHookinfo->gameMouse_x, pHookinfo->gameMouse_y);
        Print_Debug("Resolution: %u x %u\n", pHookinfo->cx, pHookinfo->cy);

		// 用完后记得释放视图（注意：句柄不要 CloseHandle！除非你自己 Duplicate 过）
		UnmapViewOfFile(pHookinfo);
       
        return true;
    }

    shared_memory_handle = nullptr;
    return false;
}

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
        10000);         // waits for 10 seconds 

    if (!fSuccess)
    {
        auto e = GetLastError();
        if (e == ERROR_MORE_DATA)
        {
            //如果数据超过buf 打印
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
