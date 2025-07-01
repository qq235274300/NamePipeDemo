#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>
#include "NamedPipeServer.h"	




int main()
{
	NamedPipeServer server;	
	server.CreateServerPipe();
	
	return 0;
}