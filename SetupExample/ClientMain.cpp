#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>
#include "NamedPipeClient.h"






int main()
{
	NamedPipeClient client;
	HANDLE shared_memory_handle = NULL; //phookinfo
	void* shmem_info = nullptr;// backbuffer
	client.send_request_get_shared_memory_handle(shared_memory_handle);

	while (true)
	{
	};
	return 0;
}