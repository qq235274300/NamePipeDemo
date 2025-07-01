#pragma once
#include <windows.h>
#include <thread>

#include "NamedPipeServer.h"

using namespace std;

//��������� 
// 1.����NamedPipeServer
// 2.��������������������Ϣ�߳�
class ServerInstance
{
public:
    ~ServerInstance();
    static ServerInstance* Instance()
    {
        if (instance == NULL)
        {
            instance = new ServerInstance();
        }
        return instance;
    };

public:
    bool initialize();
    HANDLE shared_memory_handle_get();
private:
    void server_thread_routine();

    bool check_booting_time(TCHAR* booting_time_string, int length);
private:
    TCHAR booting_time[MAX_PATH];
    TCHAR servicePipeName[MAX_PATH];

    NamedPipeServer server;
    std::thread server_thread;
    DWORD server_comm_thread_id = 0xFFFFFFFF;
    bool is_running = false;
    HANDLE shared_memory_handle;
private:
    ServerInstance()
    {
        instance = NULL;

    }

    static ServerInstance* instance;
};

