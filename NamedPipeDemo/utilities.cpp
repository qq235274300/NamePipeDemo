#include "utilities.h"
#include "SafeFunc.h"
#include <corecrt_wstdio.h>
#include "logging.h"


#define MODULE_TAG  ("ServerUtilities")
bool utilities_get_pipename(TCHAR* pipename, int pipename_length)
{
    if (pipename != nullptr)
    {
		TCHAR pipe_name[MAX_PATH] = { 0 };

		if (!SafeFunc::Instance()->ReadServicePipeName(pipe_name, MAX_PATH))
		{
			Print_Error("Failed to read pipe name from registry!\n");
			return false;
		}

		// 最安全的拼接：确保输出缓冲区足够
		int ret = swprintf_s(pipename, pipename_length, TEXT("\\\\.\\pipe\\%s"), pipe_name);
		if (ret < 0)
		{
			Print_Error("swprintf_s failed while formatting pipe name!\n");
			return false;
		}

		return true;
    }

    return false;
}
