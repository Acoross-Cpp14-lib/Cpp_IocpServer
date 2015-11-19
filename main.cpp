#include <iostream>

#include "Utility.h"
#include "Network.h"
#include "IOCP.h"

using namespace Acoross;
using Acoross::Async::CIOCP;
using namespace Acoross::Network;
using Acoross::Async::CIOObject;

CLog Acoross::Log;

using Acoross::Async::CIOThreadControlBlock;
bool IOThreadFunc(CIOThreadControlBlock* pThreadBlock)
{
	if (nullptr == pThreadBlock)
	{
		return false;
	}

	const HANDLE hIOCP = pThreadBlock->hIOCP;

	while (!pThreadBlock->bStop)
	{
		DWORD dwTransferred = 0;
		CIOObject* pObject = nullptr;
		//unsigned long key = 0;
		WSAOVERLAPPED* pOverlap = nullptr;
		if (GetQueuedCompletionStatus(hIOCP, &dwTransferred, (PULONG_PTR)&pObject, &pOverlap, INFINITE))
		{
			pObject->OnIOCallback();
			continue;
		}
		else
		{
			Log.Add(L"GetQueuedCompletionStatus failed:%d\n", GetLastError());
			break;
		}
	}

	return true;
}

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