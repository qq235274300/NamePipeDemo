#include "utilities.h"
#include "SafeFunc.h"
#include <corecrt_wstdio.h>

bool utilities_get_pipename(TCHAR* pipename, int pipename_length)
{
    if (pipename != nullptr)
    {
        TCHAR pipe_name[MAX_PATH];
        SafeFunc::Instance()->ReadServicePipeName(pipe_name, MAX_PATH);
        swprintf_s(pipename, pipename_length, TEXT("\\\\.\\pipe\\%s"), pipe_name);
        return true;
    }

    return false;
}
