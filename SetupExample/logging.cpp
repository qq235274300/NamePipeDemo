//Copyright (c) 2022 Lenovo. All right reserved.
//Confidential and Proprietary
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include "logging.h"
#include "SafeFunc.h"



#define LOG_LEVEL_UNDEFINE          200
#define LOG_LEVEL_NO_LOG_OUTPUT     100

const char LogFileName[MAX_PATH] = "NamedPipeClient_log.txt";
static char log_file_name[MAX_PATH] = "";
static bool log_file_enable = false;
static int  log_level = LOG_LEVEL_UNDEFINE;

const char RecordFileName[MAX_PATH] = "NamedPipeClient_log_record.txt";
static char record_file_name[MAX_PATH] = "";
static bool record_file_enable = false;

void my_log_print(const char* mod, int level, const char* func_name, int line_number, const char* fmt, ...)
{
    FILE* pFile = nullptr;
    SYSTEMTIME localTime;
    char log_file_buffer[1024];
    char log_file_content[1024];
    char c[] = { 'E','E','I','D' };

    if (log_level == LOG_LEVEL_UNDEFINE)
    {
        log_level = SafeFunc::Instance()->GetLogLevel();
    }

    if (level > log_level)
    {
        return;
    }

    SecureZeroMemory(log_file_buffer, sizeof(log_file_buffer));
    SecureZeroMemory(log_file_content, sizeof(log_file_content));

    va_list args;
    va_start(args, fmt);
    vsprintf_s(log_file_content, fmt, args);
    va_end(args);

    ::GetLocalTime(&localTime);

    sprintf_s(log_file_buffer, sizeof(log_file_buffer), "[%c][%04d%02d%02d-%02d:%02d:%02d.%03d][%s][%s,%d] %s\n",
        c[level], (localTime.wYear), (localTime.wMonth), localTime.wDay,
        localTime.wHour, localTime.wMinute, localTime.wSecond, (int)(localTime.wMilliseconds),
        mod, func_name, line_number, log_file_content);

    if (!log_file_enable)
    {
        TCHAR log_file_nameW[MAX_PATH];
        DWORD size = MAX_PATH;

        SecureZeroMemory(log_file_name, sizeof(log_file_name));

        if (SafeFunc::Instance()->GetProtectDirectory(log_file_nameW, size))
        {
            log_file_enable = true;

            WideCharToMultiByte(CP_ACP, 0, log_file_nameW, -1, log_file_name, sizeof(log_file_name) - 1, NULL, FALSE);
            strcat_s(log_file_name, MAX_PATH, "\\");
            strcat_s(log_file_name, MAX_PATH, LogFileName);
        }
    }

    fopen_s(&pFile, log_file_name, "at+");
    if (pFile != nullptr)
    {
        fprintf(pFile, log_file_buffer);
        fclose(pFile);
        pFile = NULL;
    }

#ifdef _DEBUG
    printf(log_file_buffer);
#endif
}


void my_record_print(const char* fmt, ...)
{
    FILE* pFile = nullptr;
    SYSTEMTIME localTime;
    char record_file_buffer[1024];
    char record_file_content[1024];

    SecureZeroMemory(record_file_buffer, sizeof(record_file_buffer));
    SecureZeroMemory(record_file_content, sizeof(record_file_content));

    va_list args;
    va_start(args, fmt);
    vsprintf_s(record_file_content, fmt, args);
    va_end(args);

    ::GetLocalTime(&localTime);

    sprintf_s(record_file_buffer, sizeof(record_file_buffer), "[%04d%02d%02d-%02d:%02d:%02d.%03d] %s",
        (localTime.wYear), (localTime.wMonth), localTime.wDay,
        localTime.wHour, localTime.wMinute, localTime.wSecond, (int)(localTime.wMilliseconds),
        record_file_content);

    if (!record_file_enable)
    {
        TCHAR record_file_nameW[MAX_PATH];
        DWORD size = MAX_PATH;

        SecureZeroMemory(record_file_name, sizeof(record_file_name));

        if (SafeFunc::Instance()->GetProtectDirectory(record_file_nameW, size))
        {
            record_file_enable = true;

            WideCharToMultiByte(CP_ACP, 0, record_file_nameW, -1, record_file_name, sizeof(record_file_name) - 1, NULL, FALSE);
            strcat_s(record_file_name, MAX_PATH, "\\");
            strcat_s(record_file_name, MAX_PATH, RecordFileName);
        }
    }

    fopen_s(&pFile, record_file_name, "at+");
    if (pFile != nullptr)
    {
        fprintf(pFile, record_file_buffer);
        fclose(pFile);
        pFile = NULL;
    }

#ifdef _DEBUG
    printf(record_file_buffer);
#endif
}