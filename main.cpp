#include <iostream>
#include <new>
#include <vector>

#include "MemoryManager.h"
#include "Utility.h"
#include "Network.h"
#include "IOCP.h"

using namespace Acoross;
using Acoross::Async::CIOCP;
using namespace Acoross::Network;
using Acoross::Async::CIOObject;

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

	//pIOCP->Clear();

	return 0;
}