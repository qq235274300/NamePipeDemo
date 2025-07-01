#pragma once
#include <cstdint>


#define PIPE_HEADER_MAGIC 0x5aa57135

enum avatar_pipe_msg_type {
	msg_type_get_shared_semaphore_handle_req = 0x0001,
	msg_type_get_shared_semaphore_handle_resp,
	msg_type_notify_multi_boot_req = 0x0003,
	msg_type_notify_multi_boot_resp,
	msg_type_get_shared_memory_handle_req = 0x0005,
	msg_type_get_shared_memory_handle_resp,
	msg_type_notify_semaphore_add_req = 0x0007,
	msg_type_notify_semaphore_add_resp,
	msg_type_notify_semaphore_dec_req = 0x0009,
	msg_type_notify_semaphore_dec_resp,
	msg_type_notify_avatarUI_exit_req = 0x000b,
	msg_type_notify_avatarUI_exit_resp,
	msg_type_notify_avatarUI_start_req = 0x000d,
	msg_type_notify_avatarUI_start_resp,
	msg_type_notify_oobeshowed_req = 0x000f,
	msg_type_notify_oobeshowed_resp,
	msg_type_notify_vcam_opened_req = 0x0011,
	msg_type_notify_vcam_opened_resp,
	msg_type_notify_vcam_closed_req = 0x0013,
	msg_type_notify_vcam_closed_resp,
};

#pragma pack(1) //强行关闭字节对齐 IPC要求
typedef struct {
    uint32_t magic;
    uint32_t type;
    uint32_t reserve1;
    uint32_t reserve2;
}PipePacketHeader;

typedef struct {
    uint64_t process_ID; //请求
}REQ_Shared_Handle;

typedef struct {
    uint64_t handle; //响应
}RESP_Shared_Handle;

typedef struct {
    PipePacketHeader hdr;
    union {
        REQ_Shared_Handle   req_shared_handle;
        RESP_Shared_Handle  resp_shared_handle;
    }u;
}CommmonPipePacket;
#pragma pack()