#pragma once
#include <cstdint>


#define PIPE_HEADER_MAGIC 0x5aa57135

#pragma pack(1) //ǿ�йر��ֽڶ��� IPCҪ��
typedef struct {
    uint32_t magic;
    uint32_t type;
    uint32_t reserve1;
    uint32_t reserve2;
}PipePacketHeader;

typedef struct {
    uint64_t process_ID; //����
}REQ_Shared_Handle;

typedef struct {
    uint64_t handle; //��Ӧ
}RESP_Shared_Handle;

typedef struct {
    PipePacketHeader hdr;
    union {
        REQ_Shared_Handle   req_shared_handle;
        RESP_Shared_Handle  resp_shared_handle;
    }u;
}CommmonPipePacket;
#pragma pack()