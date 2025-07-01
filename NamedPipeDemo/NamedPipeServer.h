#pragma once
#include <windows.h>
#include "pipe_define.h"
#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 
#define INSTANCES 1 
#define PIPE_TIMEOUT 3000
#define BUFSIZE 1024

typedef struct
{
    OVERLAPPED oOverlap; //异步 I/O（OVERLAPPED 模式） 配合 ReadFile() / WriteFile() 的重叠模式
    HANDLE hPipeInst; //当前连接的 pipe 句柄（CreateNamedPipe() 的返回值）
    char chRequest[BUFSIZE]; //客户端发来的原始请求数据
    DWORD cbRead; //实际读取的字节数（
    char chReply[BUFSIZE]; //服务端准备要发回客户端的响应数据缓冲区
    DWORD cbToWrite; //写回客户端的数据长度
    DWORD dwState;//CONNECTING_STATE: 等待客户端连接  READING_STATE: 准备读取客户端数据 WRITING_STATE: 准备把响应发给客户端
    DWORD client_process_id; //GetNamedPipeClientProcessId() 得到的连接方进程信息
    HANDLE client_process_handle; //DuplicateHandle() 或签名验证
    BOOL fPendingIO; //标志位：是否正在进行异步 I/O 操作
} PIPEINST, * LPPIPEINST;

#define PAYLOAD_LEN(s) (sizeof(s) - sizeof(PipePacketHeader)) //减去IPC协议头部长度

typedef void (*Access_PipeData)(LPPIPEINST); //回调函数

class NamedPipeServer
{
public:
    NamedPipeServer();
    ~NamedPipeServer();

    bool CreateServerPipe(Access_PipeData callback);
    void DestroyServerPipe();
    BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);

    bool PollServerPipe(DWORD timeout);
    bool IsReady();
    VOID DisconnectAndReconnect(DWORD);
    VOID ProcessAPIRequest(LPPIPEINST);
private:
    bool CheckAndProcessClientAPIRequest(DWORD timeout);

private:
    TCHAR pipeName[MAX_PATH];

    Access_PipeData cb;

    PIPEINST Pipe[INSTANCES];
    HANDLE hEvents;
    bool m_initialized = false;
};

