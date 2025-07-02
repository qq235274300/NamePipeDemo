#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>
#include "NamedPipeClient.h"






int main()
{
	NamedPipeClient client;
	HANDLE shared_memory_handle = NULL; //phookinfo
	HANDLE shared_texture_handle = NULL;
	void* shmem_info = nullptr;// backbuffer
	client.send_request_get_shared_memory_handle(shared_memory_handle);
	client.send_request_get_shared_texture_handle(shared_texture_handle);
	while (true)
	{
	};
	return 0;
}