#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>
#include "NamedPipeClient.h"



int main()
{
	NamedPipeClient client;
	client.send_request_notify_bool();

	return 0;
}