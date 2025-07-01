#pragma once
#include <windows.h>
class SafeFunc
{
public:
    ~SafeFunc()
    {
        if (sdk != nullptr)
        {
            delete sdk;
            sdk = nullptr;
        }
    }

    static SafeFunc* Instance()
    {
        if (sdk == nullptr)
        {
            sdk = new SafeFunc();
        }
        return sdk;
    };

public:
    bool GetProtectDirectory(TCHAR* path, DWORD& size); 
    int  GetLogLevel();
    bool GetCurrentProcessName(TCHAR* processName, DWORD& size);

    bool CreateACLRules(SECURITY_ATTRIBUTES* sec);
    bool CheckACLRules(const TCHAR* Path);

    BOOL ReadRegisterValueString(const TCHAR* path, const TCHAR* item, TCHAR* valueStirng, DWORD valueSize);
    BOOL ReadServicePipeName(TCHAR* pipeName, DWORD nameSize);

    static void access_pipedata(LPPIPEINST);
private:
    SafeFunc()
    {
        sdk = nullptr;
        SafeFunc::initialed = FALSE;
    };

    static SafeFunc* sdk;
    static bool initialed;
};

