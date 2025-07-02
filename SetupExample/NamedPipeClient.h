#pragma once
#include <windows.h>
#include "pipe_define.h"

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


struct shtex_data
{
	uint32_t tex_handle;
	uint32_t capture_processid = -1000000;
};



class NamedPipeClient
{
public:
    bool send_request_notify_bool();
	//获取phookinfo共享内存句柄
    bool send_request_get_shared_memory_handle(HANDLE& shared_memory_handle);
	//获取游戏画面共享纹理句柄
	bool send_request_get_shared_texture_handle(HANDLE& shared_texture_handle);
public:
    bool send_request(bool to_service, uint8_t* req, int req_len, CommmonPipePacket* resp = nullptr);


private:
	hook_info* pHookinfo = nullptr;
	void* sharedmem_info = nullptr;
public:
    TCHAR pipeName[2][MAX_PATH] = { L"" };
    bool pipeName_valid = false;

    CommmonPipePacket resp;
};

