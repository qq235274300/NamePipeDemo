#include "NamedPipeServer.h"
#include "SafeFunc.h"
#include "utilities.h"
#include "logging.h"
#define MODULE_TAG  ("NamedPipeServer")

NamedPipeServer::NamedPipeServer()
{
    for (int i = 0; i < INSTANCES; i++) {
        Pipe[i].fPendingIO = FALSE;
        Pipe[i].hPipeInst = INVALID_HANDLE_VALUE;
        Pipe[i].dwState = CONNECTING_STATE;
        Pipe[i].oOverlap.hEvent = nullptr;
    }
    cb = nullptr;
}

NamedPipeServer::~NamedPipeServer()
{
    if (m_initialized)
    {
        DestroyServerPipe();
    }

}

bool NamedPipeServer::CreateServerPipe(Access_PipeData callback)
{
    SECURITY_ATTRIBUTES se;
    //创建 ACL 规则，管道使用权限
    if (!SafeFunc::Instance()->CreateACLRules(&se)) //访问控制描述符
    {
        Print_Error("pipe server create ACL failed!\n");
        return false;
    }
    if (!utilities_get_pipename(pipeName, MAX_PATH))//从注册表读出管道名称 保存下来
    {
        Print_Error("pipe server get pipe name failed!\n");
        return false;
    }

    //注册回调函数指针
    cb = callback;

    for (int i = 0; i < INSTANCES; i++)
    {
        // Create an event object for this instance. 
        hEvents = CreateEvent(
            NULL,    // default security attribute 
            TRUE,    // manual-reset event 
            TRUE,    // initial state = signaled 
            NULL);   // unnamed event object 

        if (hEvents == NULL)
        {
            //Print_Error("CreateEvent failed with %d.\n", GetLastError());
            return false;
        }
        //ReadFile / WriteFile 会用这个事件等待完成
        Pipe[i].oOverlap.hEvent = hEvents;
        //创建命名管道实例
        Pipe[i].hPipeInst = CreateNamedPipe(
            pipeName,            // pipe name 
            PIPE_ACCESS_DUPLEX |     // read/write access 
            FILE_FLAG_OVERLAPPED,    // overlapped mode 
            PIPE_TYPE_MESSAGE |      // message-type pipe 
            PIPE_REJECT_REMOTE_CLIENTS | //reject remote connecting
            PIPE_READMODE_MESSAGE |  // message-read mode 
            PIPE_WAIT,               // blocking mode 
            INSTANCES,               // number of instances 
            BUFSIZE * sizeof(TCHAR),   // output buffer size 
            BUFSIZE * sizeof(TCHAR),   // input buffer size 
            PIPE_TIMEOUT,            // client time-out 
            &se);                   // default security attributes 

        if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE)
        {
            char tmpString[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, pipeName, -1, tmpString, sizeof(tmpString) - 1, NULL, FALSE);
            //Print_Error("No.%d %s CreateNamedPipe failed with %d.\n", i, tmpString, GetLastError());
            return false;
        }

        // Call the subroutine to connect to the new client
        //连接客户端
        Pipe[i].fPendingIO = ConnectToNewClient(
            Pipe[i].hPipeInst,
            &Pipe[i].oOverlap);

        Pipe[i].dwState = Pipe[i].fPendingIO ?
            CONNECTING_STATE : // still connecting 
            READING_STATE;     // ready to read 
    }
    LocalFree(se.lpSecurityDescriptor);
    m_initialized = true;

    return true;
}

void NamedPipeServer::DestroyServerPipe()
{
    if (!m_initialized)
        return;
    for (int i = 0; i < INSTANCES; i++)
    {
        if (Pipe[i].hPipeInst != nullptr)
        {
            DisconnectNamedPipe(Pipe[i].hPipeInst);
            CloseHandle(Pipe[i].hPipeInst);
            Pipe[i].hPipeInst = nullptr;
        }
    }//for (int i = 0; i < INSTANCES; i++)
    if (hEvents != nullptr)
    {
        CloseHandle(hEvents);
        hEvents = nullptr;
    }
    m_initialized = false;
}

BOOL NamedPipeServer::ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
    BOOL fConnected, fPendingIO = FALSE;

    // Start an overlapped connection for this pipe instance. 
    fConnected = ConnectNamedPipe(hPipe, lpo);

    // Overlapped ConnectNamedPipe should return zero. 
    if (fConnected)
    {
        //Print_Error("ConnectNamedPipe failed with %d.\n", GetLastError());
        return FALSE;
    }

    switch (GetLastError())
    {
        // The overlapped connection in progress. 
    case ERROR_IO_PENDING:
        fPendingIO = TRUE;
        break;

        // Client is already connected, so signal an event. 
    case ERROR_PIPE_CONNECTED:
        if (SetEvent(lpo->hEvent))
            break;

        // If an error occurs during the connect operation... 
    default:
    {
        // Print_Error("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }
    }

    return fPendingIO;
}

bool NamedPipeServer::PollServerPipe(DWORD timeout)
{
    if (CheckAndProcessClientAPIRequest(timeout))
    {
        return true;
    }
    return false;
}

bool NamedPipeServer::IsReady()
{
    return m_initialized;
}

VOID NamedPipeServer::DisconnectAndReconnect(DWORD i)
{
    // Disconnect the pipe instance. 
    if (!DisconnectNamedPipe(Pipe[i].hPipeInst))
    {
        Print_Error("DisconnectNamedPipe failed with %d.\n", GetLastError());
    }

    // Call a subroutine to connect to the new client. 

    Pipe[i].fPendingIO = ConnectToNewClient(
        Pipe[i].hPipeInst,
        &Pipe[i].oOverlap);

    Pipe[i].dwState = Pipe[i].fPendingIO ?
        CONNECTING_STATE : // still connecting 
        READING_STATE;     // ready to read 
}

VOID NamedPipeServer::ProcessAPIRequest(LPPIPEINST pipe)
{
    if (cb != nullptr)
    {
        cb(pipe);
    }
}

bool NamedPipeServer::CheckAndProcessClientAPIRequest(DWORD timeout)
{
    BOOL fSuccess;
    DWORD dwWait = WaitForSingleObject(hEvents, timeout);

    if (dwWait != WAIT_OBJECT_0)
    {
        return 0;
    }

    DWORD i = 0;

    // Get the result if the operation was pending. 
    if (Pipe[i].fPendingIO)
    {
        DWORD cbRet = 0;
        fSuccess = GetOverlappedResult(
            Pipe[i].hPipeInst, // handle to pipe 
            &Pipe[i].oOverlap, // OVERLAPPED structure 
            &cbRet,            // bytes transferred 
            FALSE);            // do not wait 

        switch (Pipe[i].dwState)
        {
            // Pending connect operation 
        case CONNECTING_STATE:
        {
            bool is_connection_valid = false;

            if (!fSuccess)
            {
                Print_Error("Error %d\n", GetLastError());
                ResetEvent(hEvents);
                return false;
            }

            // Publish profile will check the peer sign
            DWORD clientPID;
            GetNamedPipeClientProcessId(Pipe[i].hPipeInst, &clientPID);
            HANDLE hTargetHandle = OpenProcess(PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION, FALSE, clientPID);
            if (hTargetHandle == nullptr) {
                Print_Error("open process failed! errCode = %d\n", GetLastError());
                ResetEvent(hEvents);
                return false;
            }

            //start to check the peer process image data sign. 
            //because we must support third party application, only check the data signature, not necessary lenovo signature
            TCHAR szClientImagePath[MAX_PATH];
            DWORD pathSize = MAX_PATH;
            SecureZeroMemory(szClientImagePath, sizeof(szClientImagePath));
            QueryFullProcessImageName(hTargetHandle, 0, szClientImagePath, &pathSize);
#if defined(PUBLISH)
            //check the image signature, not necessary lenovo signature.
            if (SafeFunc::Instance()->CheckExcutableFileValidationWithLooseAuthority(szClientImagePath)) {
                is_connection_valid = true;
            }
            else
            {
                //not our program
                is_connection_valid = false;
            }
#else
            is_connection_valid = true;
            //Pipe[i].dwState = READING_STATE;
#endif
            if (is_connection_valid)
            {
                Pipe[i].client_process_handle = hTargetHandle;
                Pipe[i].client_process_id = clientPID;
                Print_Debug("pipe server receive new connecting, handle = 0x%x\n", hTargetHandle);
                Pipe[i].dwState = READING_STATE;
            }
            else
            {
#ifndef PUBLISH
                {
                    char tmpString[MAX_PATH];
                    WideCharToMultiByte(CP_ACP, 0, szClientImagePath, -1, tmpString, sizeof(tmpString) - 1, NULL, FALSE);
                    Print_Error("pipe server receive invalid connecting, handle = 0x%x, image = %s\n\n", hTargetHandle, tmpString);
                }
#endif //PUBLISH
                CloseHandle(hTargetHandle);
                DisconnectAndReconnect(i);
                return false;
            }

            break;
        }
        // Pending read operation 
        case READING_STATE:
        {
            if (!fSuccess || cbRet == 0)
            {
                DisconnectAndReconnect(i);
                return false;
            }
            Pipe[i].cbRead = cbRet;
            Pipe[i].dwState = WRITING_STATE;
            break;
        }
        // Pending write operation 
        case WRITING_STATE:
        {
            if (!fSuccess || cbRet != Pipe[i].cbToWrite)
            {
                DisconnectAndReconnect(i);
                return false;
            }
            Pipe[i].dwState = READING_STATE;
            break;
        }
        default:
        {
            Print_Error("After wait, invalid pipe state %d, pipe %d.\n", Pipe[i].dwState, i);
            return 0;
        }
        }//switch (Pipe[i].dwState)
    }

    // The pipe state determines which operation to do next. 

    switch (Pipe[i].dwState)
    {
        // READING_STATE: 
        // The pipe instance is connected to the client 
        // and is ready to read a request from the client. 

    case READING_STATE:
    {
        fSuccess = ReadFile(
            Pipe[i].hPipeInst,
            Pipe[i].chRequest,
            BUFSIZE * sizeof(TCHAR),
            &Pipe[i].cbRead,
            &Pipe[i].oOverlap);

        // The read operation completed successfully. 
        if (fSuccess && Pipe[i].cbRead != 0)
        {
            Pipe[i].fPendingIO = FALSE;
            //LOG_F(INFO, "Read %u bytes", Pipe[i].cbRead);
            Pipe[i].dwState = WRITING_STATE;
            break;
        }

        // The read operation is still pending. 

        DWORD dwErr = GetLastError();
        if (!fSuccess && (dwErr == ERROR_IO_PENDING))
        {
            Pipe[i].fPendingIO = TRUE;
            break;
        }

        // An error occurred; disconnect from the client. 
        DisconnectAndReconnect(i);
        break;
    }
    // WRITING_STATE: 
    // The request was successfully read from the client. 
    // Get the reply data and write it to the client. 
    case WRITING_STATE:
    {
        ProcessAPIRequest(&Pipe[i]);
        DWORD cbRet = 0;
        fSuccess = WriteFile(
            Pipe[i].hPipeInst,
            Pipe[i].chReply,
            Pipe[i].cbToWrite,
            &cbRet,
            &Pipe[i].oOverlap);

        // The write operation completed successfully. 

        if (fSuccess && cbRet == Pipe[i].cbToWrite)
        {
            Pipe[i].fPendingIO = FALSE;
            Pipe[i].dwState = READING_STATE;
            break;
        }

        // The write operation is still pending. 
        DWORD dwErr = GetLastError();
        if (!fSuccess && (dwErr == ERROR_IO_PENDING))
        {
            Pipe[i].fPendingIO = TRUE;
            break;
        }
        // An error occurred; disconnect from the client. 
        DisconnectAndReconnect(i);
        break;
    }
    default:
    {
        Print_Error("Io complete, invalid pipe state %d, pipe %d.\n", Pipe[i].dwState, i);
        ResetEvent(hEvents);
        return 0;
    }
    }
    return true;
}

