#include "ServerInstance.h"
#include "logging.h"
#include "SafeFunc.h"

#define MODULE_TAG  ("ServerInstance")
ServerInstance* ServerInstance::instance = NULL;

ServerInstance::~ServerInstance()
{

}

bool ServerInstance::initialize()
{
    GUID guid;
    int randNumber = 0;

    SecureZeroMemory(servicePipeName, sizeof(servicePipeName));

    srand((unsigned int)time(NULL));

    ::CoCreateGuid(&guid);
    swprintf_s(servicePipeName, MAX_PATH, TEXT("%02X%02X%02X%02X%02X%02X%02X22"),
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6]);

    SafeFunc::Instance()->UpdateServicePipeName(servicePipeName, MAX_PATH);
    SafeFunc::Instance()->WriteRegisterValueString(TEXT("SOFTWARE\\Lenovo\\Bino3D"), TEXT("booting_time"), booting_time, MAX_PATH);

    is_running = true;
    server_thread = std::thread(&ServerInstance::server_thread_routine, this);
    if (!server_thread.joinable())
    {
        Print_Error("Fatal error, cannot creat the server_thread_routine thread!\n");
        return false;
    }
    else
    {

    }

    return true;
}

HANDLE ServerInstance::shared_memory_handle_get()
{
    return shared_memory_handle;
}

void ServerInstance::server_thread_routine()
{
    server_comm_thread_id = GetCurrentThreadId();

    if (!server.CreateServerPipe(SafeFunc::access_pipedata))
    {
        Print_Error("failed to create pipe server!\n");
        return;
    }
    Print_Debug("create server pipe OK!\n");

    while (is_running)
    {
        if (server.IsReady())
        {
            server.PollServerPipe(1000);
        }

    }

}

bool ServerInstance::check_booting_time(TCHAR* booting_time_string, int length)
{
    TCHAR reg_booting_time_string[MAX_PATH];
    SYSTEMTIME currentTime;
    FILETIME ftLocalStartTime;
    FILETIME ftCurrentTime, ftStartTime;
    ULONGLONG uptimeSeconds = GetTickCount64() * 10000ULL;


    GetSystemTime(&currentTime);

    SystemTimeToFileTime(&currentTime, &ftCurrentTime);

    ULONGLONG startTimeFileTime = ftCurrentTime.dwHighDateTime;
    startTimeFileTime = startTimeFileTime << 32;
    startTimeFileTime += ftCurrentTime.dwLowDateTime;
    startTimeFileTime -= uptimeSeconds;

    ftStartTime.dwLowDateTime = static_cast<DWORD>(startTimeFileTime & 0xFFFFFFFF);
    ftStartTime.dwHighDateTime = static_cast<DWORD>(startTimeFileTime >> 32);


    FileTimeToLocalFileTime(&ftStartTime, &ftLocalStartTime);
    FileTimeToSystemTime(&ftLocalStartTime, &currentTime);

    swprintf_s(booting_time_string, length, TEXT("%04d%02d%02d%02d%02d%02d"), currentTime.wYear, currentTime.wMonth, currentTime.wDay, currentTime.wHour, currentTime.wMinute, currentTime.wSecond);

    if (!SafeFunc::Instance()->ReadRegisterValueString(TEXT("SOFTWARE\\Lenovo\\Bino3D"), TEXT("booting_time"), reg_booting_time_string, MAX_PATH))
    {
        return true;
    }

    if (wcscmp(booting_time_string, reg_booting_time_string) != 0)
    {
        return true;
    }

    return false;
}
