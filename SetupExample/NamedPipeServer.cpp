#include "NamedPipeServer.h"
#include "SafeFunc.h"
#include "utilities.h"

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
    //start to create the pipe name
    if (!SafeFunc::Instance()->CreateACLRules(&se)) //访问控制描述符
    {
        //Print_Error("pipe server create ACL failed!\n");
        return false;
    }
    if (!utilities_get_pipename(pipeName, MAX_PATH))//从注册表读出管道名称 保存下来
    {
        //Print_Error("pipe server get pipe name failed!\n");
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

