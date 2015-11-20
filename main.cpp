#include <iostream>

#include "Utility.h"
#include "Network.h"
#include "IOCP.h"

using namespace Acoross;
using Acoross::Async::CIOCP;
using namespace Acoross::Network;
using Acoross::Async::CIOObject;

CLog Acoross::Log;

int main(int argc, char* argv[])
{
	///////////////////
	//// IOCP
	//auto pIOCP = std::make_shared<CIOCP<8>>();
	//if (!pIOCP->CreateIOCP())
	//{	
	//	return 1;
	//}
	//else
	//{
	//	if (!pIOCP->BeginIOThreadPool(IOThreadFunc))
	//	{
	//		pIOCP->Clear();
	//		return 1;
	//	}
	//	//else
	//	{

	//	}
	//}
	
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