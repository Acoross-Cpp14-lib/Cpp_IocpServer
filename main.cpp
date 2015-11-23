#include <iostream>

#include "Utility.h"
#include "Network.h"
#include "IOCP.h"
#include "Server.h"

using namespace Acoross;

Acoross::CLog Acoross::Log;

int main(int argc, char* argv[])
{	
	/////////////////
	// Server
	if (!Network::Init())
	{
		return 1;
	}
	else
	{
		CServer server;
		server.Run();

		Network::Cleanup();
	}

	return 0;
}