#pragma once
#include <windows.h>
#include "pipe_define.h"
class NamedPipeClient
{
public:
    bool send_request_notify_bool();

public:
    bool send_request(bool to_service, uint8_t* req, int req_len, CommmonPipePacket* resp = nullptr);

public:
    TCHAR pipeName[2][MAX_PATH] = { L"" };
    bool pipeName_valid = false;
    
    CommmonPipePacket resp;
};

