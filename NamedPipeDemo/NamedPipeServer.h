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
    OVERLAPPED oOverlap; //�첽 I/O��OVERLAPPED ģʽ�� ��� ReadFile() / WriteFile() ���ص�ģʽ
    HANDLE hPipeInst; //��ǰ���ӵ� pipe �����CreateNamedPipe() �ķ���ֵ��
    char chRequest[BUFSIZE]; //�ͻ��˷�����ԭʼ��������
    DWORD cbRead; //ʵ�ʶ�ȡ���ֽ�����
    char chReply[BUFSIZE]; //�����׼��Ҫ���ؿͻ��˵���Ӧ���ݻ�����
    DWORD cbToWrite; //д�ؿͻ��˵����ݳ���
    DWORD dwState;//CONNECTING_STATE: �ȴ��ͻ�������  READING_STATE: ׼����ȡ�ͻ������� WRITING_STATE: ׼������Ӧ�����ͻ���
    DWORD client_process_id; //GetNamedPipeClientProcessId() �õ������ӷ�������Ϣ
    HANDLE client_process_handle; //DuplicateHandle() ��ǩ����֤
    BOOL fPendingIO; //��־λ���Ƿ����ڽ����첽 I/O ����
} PIPEINST, * LPPIPEINST;

#define PAYLOAD_LEN(s) (sizeof(s) - sizeof(PipePacketHeader)) //��ȥIPCЭ��ͷ������

typedef void (*Access_PipeData)(LPPIPEINST); //�ص�����

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

