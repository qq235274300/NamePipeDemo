#include "SafeFunc.h"
#include <sddl.h>
#include <aclapi.h>
#include <AccCtrl.h>
#include <shlobj_core.h>
#include "logging.h"
#include "ServerInstance.h"
#define MODULE_TAG  ("SeverSafeFunc")

SafeFunc* SafeFunc::sdk = nullptr;
bool SafeFunc::initialed = FALSE;

#define ACLEntryNumber  5
#define ACCESS_ALLOW    0
#define ACCESS_DENY     1
#define PERMISSIONMASK  0x1F

typedef struct
{
    TCHAR onwerName[MAX_PATH];
    TCHAR ownerGroup[MAX_PATH];
    int   accessFlag;
    unsigned long mask;
    unsigned int setMask;
}ACLCheckList;

static ACLCheckList aclList[ACLEntryNumber] =
{
    {TEXT(""), TEXT(""), ACCESS_DENY, DELETE | FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE | READ_CONTROL | WRITE_DAC | WRITE_OWNER | SYNCHRONIZE, 0x01},
    {TEXT(""), TEXT(""), ACCESS_DENY, DELETE | FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE | READ_CONTROL | WRITE_DAC | WRITE_OWNER | SYNCHRONIZE, 0x02},
    {TEXT(""), TEXT(""), ACCESS_ALLOW, FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE | READ_CONTROL | SYNCHRONIZE, 0x04},
    {TEXT("CREATOR OWNER"), TEXT(""), ACCESS_ALLOW, FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE | READ_CONTROL | SYNCHRONIZE, 0x08},
    {TEXT("CREATOR OWNER"), TEXT(""), ACCESS_ALLOW, DELETE | FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE | READ_CONTROL | WRITE_DAC | WRITE_OWNER | SYNCHRONIZE, 0x10}
};

bool SafeFunc::GetProtectDirectory(TCHAR* path, DWORD& size)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szImageName[MAX_PATH];
    DWORD imageNameSize = MAX_PATH;
    bool ret = true;
    bool existFlag = false;

    //get %LocalAppData% directory
    SecureZeroMemory(szPath, sizeof(szPath));
    if (FAILED(SHGetFolderPath(NULL,
        CSIDL_LOCAL_APPDATA,
        NULL,
        0,
        szPath)))
    {
        return false;
    }

    //combine the private directory name
    wcscat_s(szPath, MAX_PATH, TEXT("\\Bino3D"));
    if (wcsnlen_s(szPath, MAX_PATH) > size)
    {
        return false;
    }
    wcscpy_s(path, size, szPath);

    //check the directory whether exist
    existFlag = false;
    {
        WIN32_FIND_DATA fd = { 0 };
        HANDLE hFile = FindFirstFile(szPath, &fd);

        if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                existFlag = true;
            }
            FindClose(hFile);
        }
    }

    //create the directory if not exist
    if (!existFlag)
    {
        if (!CreateDirectory(szPath, NULL))
        {
            return false;
        }
    }

    //get the image name
    if (GetCurrentProcessName(szImageName, imageNameSize))
    {
        TCHAR* ptr1 = NULL, * ptr2 = NULL;

        ptr1 = szImageName;
        ptr2 = ptr1;

        while (ptr2 != NULL)
        {
            ptr2 = wcsstr(ptr1, TEXT("\\"));
            if (ptr2 != NULL)
            {
                ptr1 = ptr2 + 1;
            }
        }
        wcscpy_s(szImageName, MAX_PATH, ptr1);
    }

    //combine the name
    wcscat_s(szPath, MAX_PATH, TEXT("\\"));
    wcscat_s(szPath, MAX_PATH, szImageName);
    if (wcsnlen_s(szPath, MAX_PATH) > size)
    {
        return false;
    }
    wcscpy_s(path, size, szPath);
    size = (DWORD)wcsnlen_s(szPath, MAX_PATH);

    //check the directory whether exist
    existFlag = false;
    {
        WIN32_FIND_DATA fd = { 0 };
        HANDLE hFile = FindFirstFile(szPath, &fd);

        if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                existFlag = true;
            }
            FindClose(hFile);
        }
    }

    //create the directory if not exist
    if (!existFlag)
    {
        if (!CreateDirectory(szPath, NULL))
        {
            ret = false;
        }
    }

    return ret;
}

int SafeFunc::GetLogLevel()
{
#ifdef PUBLISH
    int value = -1;

    //value = ReadRegisterValueDWORD(TEXT("SOFTWARE\\Lenovo\\Bino3D"), TEXT("enableLog"));
    value = 0; //Publish 模式下默认不输出日志
    if (value <= 0)
    {
        return 0;
    }
    else
    {
        return 5;
    }
#else
#ifdef _DEBUG
    return 5;
#else
    return 3;
#endif
#endif
}


bool SafeFunc::GetCurrentProcessName(TCHAR* processName, DWORD& size)
{
    bool ret = false;
    TCHAR currentProcessName[MAX_PATH];
    //TCHAR* p1 = NULL, * p2 = NULL;
    DWORD  pathSize = MAX_PATH;

    if (QueryFullProcessImageName(GetCurrentProcess(), 0, currentProcessName, &pathSize))
    {
        pathSize = (DWORD)wcsnlen_s(currentProcessName, MAX_PATH);
        if (pathSize < size)
        {
            wcscpy_s(processName, size, currentProcessName);
            size = pathSize;
            ret = TRUE;
        }
    }//if (QueryFullProcessImageName(GetCurrentProcess(), 0, currentProcessPath, &pathSize))

    return ret;
}

bool SafeFunc::CreateACLRules(SECURITY_ATTRIBUTES* sec)
{
    bool ret = false;
    TCHAR data[128] = TEXT("D:(D;OICI;GA;;;BG)(D;OICI;GA;;;AN)(A;OICI;GRGWGX;;;AU)(A;OICI;GRGWGX;;;CO)(A;OICI;GA;;;BA)");

    if (sec == NULL) return false;

    SecureZeroMemory(sec, sizeof(SECURITY_ATTRIBUTES));
    sec->nLength = sizeof(SECURITY_ATTRIBUTES);
    ret = ConvertStringSecurityDescriptorToSecurityDescriptor(
        data,
        SDDL_REVISION_1,
        &sec->lpSecurityDescriptor,
        NULL);

    return ret;
}

bool SafeFunc::CheckACLRules(const TCHAR* Path)
{
    PSECURITY_DESCRIPTOR psd = NULL;
    PACL pdacl;
    ACL_SIZE_INFORMATION aclSize = { 0 };
    PSID sidowner = NULL;
    PSID sidgroup = NULL;
    TCHAR oname[MAX_PATH], doname[MAX_PATH];
    DWORD namelen, domainnamelen;
    SID_NAME_USE peUse;
    ACCESS_ALLOWED_ACE* ace;
    int aclIndex = 0, index = 0;
    SID* sid = NULL;
    unsigned long mask;
    int accessFlag;
    unsigned int permissionMask = 0;

    if (ERROR_SUCCESS != GetNamedSecurityInfo(Path
        , SE_FILE_OBJECT
        , OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION
        , &sidowner
        , &sidgroup
        , &pdacl
        , NULL
        , &psd))
    {
        return false;
    }

    //get current use name and group
    LookupAccountSid(NULL, sidowner, oname, &namelen, doname, &domainnamelen, &peUse);
    wcscpy_s(aclList[0].onwerName, MAX_PATH, oname);
    wcscpy_s(aclList[1].onwerName, MAX_PATH, oname);
    wcscpy_s(aclList[2].onwerName, MAX_PATH, oname);
    wcscpy_s(aclList[0].ownerGroup, MAX_PATH, doname);
    wcscpy_s(aclList[1].ownerGroup, MAX_PATH, doname);
    wcscpy_s(aclList[2].ownerGroup, MAX_PATH, doname);

    for (aclIndex = 0; aclIndex < (*pdacl).AceCount; aclIndex++)
    {
        if (!GetAce(pdacl, aclIndex, (PVOID*)&ace))
        {
            continue;
        }

        if (((ACCESS_ALLOWED_ACE*)ace)->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
        {
            sid = (SID*)&((ACCESS_ALLOWED_ACE*)ace)->SidStart;
            LookupAccountSid(NULL, sid, oname, &namelen, doname, &domainnamelen, &peUse);
            mask = ((ACCESS_ALLOWED_ACE*)ace)->Mask;
            accessFlag = ACCESS_ALLOW;
        }
        else if (((ACCESS_DENIED_ACE*)ace)->Header.AceType == ACCESS_DENIED_ACE_TYPE)
        {
            sid = (SID*)&((ACCESS_DENIED_ACE*)ace)->SidStart;
            LookupAccountSid(NULL, sid, oname, &namelen, doname, &domainnamelen, &peUse);
            mask = ((ACCESS_DENIED_ACE*)ace)->Mask;
            accessFlag = ACCESS_DENY;
        }
        else
        {
            continue;
        }

        for (index = 0; index < ACLEntryNumber; index++)
        {
            if (accessFlag == aclList[index].accessFlag
                && wcscmp(doname, aclList[index].ownerGroup) == 0
                && wcscmp(oname, aclList[index].onwerName) == 0
                && (mask & aclList[index].mask) == aclList[index].mask)
            {
                permissionMask |= aclList[index].setMask;
            }
        }
    }//for (aclIndex = 0; aclIndex < (*pdacl).AceCount; aclIndex++)

    if (permissionMask == PERMISSIONMASK)
    {
        return true;
    }

    return false;
}

BOOL SafeFunc::UpdatePipeName(const TCHAR* pipeName, DWORD size)
{
    return WriteRegisterValueString(TEXT("SOFTWARE\\Lenovo\\Bino3D"), TEXT("PipeName"), pipeName, size);
}

BOOL SafeFunc::UpdateServicePipeName(const TCHAR* pipeName, DWORD size)
{
    WriteRegisterValueString(TEXT("Software\\Lenovo\\Bino3D"), TEXT("ServicePipeName"), pipeName, size);
    WriteRegisterValueString(TEXT("Software\\WOW6432Node\\Lenovo\\Bino3D"), TEXT("ServicePipeName"), pipeName, size);
    return TRUE;
}

BOOL SafeFunc::WriteRegisterValueString(const TCHAR* path, const TCHAR* item, const TCHAR* valueStirng, DWORD valueSize)
{
    HKEY hKey = nullptr, hTempKey = nullptr;
    BOOL ret = FALSE;
    //HKEY_CURRENT_USER  HKEY_LOCAL_MACHINE 需管理员权限
    if (RegOpenKeyEx(HKEY_CURRENT_USER, NULL, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        if (ERROR_SUCCESS == RegCreateKey(hKey, path, &hTempKey))
        {
            if (ERROR_SUCCESS == RegSetValueEx(hTempKey, item, 0, REG_SZ, (CONST BYTE*)valueStirng, (DWORD)wcsnlen_s(valueStirng, valueSize) * 2))
            {
                ret = TRUE;
            }
            RegCloseKey(hTempKey);
        }
        RegCloseKey(hKey);
    }
    return ret;
}


BOOL SafeFunc::ReadRegisterValueString(const TCHAR* path, const TCHAR* item, TCHAR* valueStirng, DWORD valueSize)
{
    HKEY hKey = nullptr;
    unsigned char szBuffer[MAX_PATH];
    BOOL result = FALSE;
    DWORD dwBufferSize;
    //HKEY_LOCAL_MACHINE 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        SecureZeroMemory(&szBuffer[0], sizeof(szBuffer));

        dwBufferSize = sizeof(szBuffer);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, item, 0, NULL, (LPBYTE)szBuffer, &dwBufferSize))
        {
            if (dwBufferSize > valueSize)
            {
                dwBufferSize = valueSize - 1;
            }
            memcpy_s(valueStirng, valueSize, szBuffer, dwBufferSize);
            result = TRUE;
        }
        RegCloseKey(hKey);
    }

    return result;
}

BOOL SafeFunc::ReadServicePipeName(TCHAR* pipeName, DWORD nameSize)
{
    return ReadRegisterValueString(TEXT("Software\\Lenovo\\Bino3D"), TEXT("ServicePipeName"), pipeName, nameSize);
}

void SafeFunc::access_pipedata(LPPIPEINST data)
{
    PipePacketHeader* hdr = (PipePacketHeader*)&data->chRequest[0];
    CommmonPipePacket* package = (CommmonPipePacket*)&data->chReply[0];

    package->hdr.magic = PIPE_HEADER_MAGIC;

    switch (hdr->type)
    {

    case msg_type_get_shared_memory_handle_req:
    {
        HANDLE handle = nullptr;
        package->hdr.type = msg_type_get_shared_memory_handle_resp;
        if (!DuplicateHandle(GetCurrentProcess(), ServerInstance::Instance()->shared_memory_handle_get(), data->client_process_handle, &handle, 0, FALSE, DUPLICATE_SAME_ACCESS))
        {
            Print_Error("duplicate the shared memory handle failed, err = %d\n", GetLastError());
            package->u.resp_shared_handle.handle = 0;
        }
        else
        {
            package->u.resp_shared_handle.handle = (uint64_t)handle;
        }
        Print_Debug("pipe server receive msg_type_get_shared_memory_handle_req and response shared memory handle = 0x%x\n", package->u.resp_shared_handle.handle);
        break;
    }

    default:
        break;
    }
    data->cbToWrite = sizeof(CommmonPipePacket);
}

