#include <iostream>
#include <new>
#include <vector>

#include "Utility.h"
#include "Network.h"
#include "IOCP.h"
#include "Server.h"

using namespace Acoross;

int main(int argc, char* argv[])
{	
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