#pragma once
#include <windows.h>
#include <thread>

#include "NamedPipeServer.h"

using namespace std;

//这个单列类 
// 1.创建NamedPipeServer
// 2.用于启动服务器接受消息线程

#pragma pack(push, 8)

enum capture_type
{
	CAPTURE_TYPE_MEMORY,
	CAPTURE_TYPE_TEXTURE,
};
#pragma pack(push, 8)
struct hook_info
{
	/* hook version */
	uint32_t hook_ver_major;
	uint32_t hook_ver_minor;

	/* mouse position */
	float gameMouse_x;
	float gameMouse_y;
	bool showDrawCursor;
	uint8_t padding0[3];
	/* capture info */
	capture_type type;
	uint32_t window;
	uint32_t format;
	uint32_t cx;
	uint32_t cy;
	uint32_t UNUSED_base_cx;
	uint32_t UNUSED_base_cy;
	uint32_t pitch;
	uint32_t map_id;
	uint32_t map_size;
	uint32_t deviceapi_id;
	bool flip;
	uint8_t padding1[3];
	/* additional options */
	uint64_t frame_interval;
	bool UNUSED_use_scale;
	bool force_shmem = false; //默认使用共享纹理
	bool capture_overlay;
	bool allow_srgb_alias;
	uint8_t padding2[3];

	uint8_t reserved[567];
};
//648 +8 +1
static_assert(sizeof(hook_info) == 656);

#pragma pack(pop)


extern hook_info* pHookInfo;


//游戏画面
struct shtex_data
{
	uint32_t tex_handle;
	uint32_t capture_processid;
};




class ServerInstance
{
public:
    ~ServerInstance();

	bool InitializeHookInfoSharedMemory();
	bool InitializeHookInfoSharedTexture();
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
	HANDLE shared_texture_handle_get();
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
    HANDLE shared_memory_handle =NULL;

	HANDLE shared_texture_handle = NULL;
	shtex_data* pShtex = nullptr;
private:
    ServerInstance()
    {
        instance = NULL;

    }

    static ServerInstance* instance;
};

