#include "Server.h"
#include "NetworkDefault.h"

#include <Windows.h>
#include <utility>
#include <functional>

#include "Utility.h"
#include "listener.h"
#include "AsyncSocket.h"

using namespace Acoross::Network::Async;

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
			if (pObject && false == pObject->OnIOCallback(dwTransferred, pOverlap))
			{
				delete pObject;
			}

			continue;
		}
		else
		{
			Log.Add(L"GetQueuedCompletionStatus failed:%d\n", GetLastError());
			if (pObject && false == pObject->OnIOCallback(dwTransferred, pOverlap))
			{
				delete pObject;
			}

			continue;
		}
	}

	return true;
}
			
void CServer::Run()
{
	using Acoross::Network::CSocketListener;

	/////////////////
	// IOCP
	auto pIOCP = std::make_shared<CIOCP<8>>();
	if (!pIOCP->CreateIOCP())
	{
		return;
	}
	else
	{
		if (!pIOCP->BeginIOThreadPool(IOThreadFunc))
		{
			pIOCP->Clear();
			return;
		}
		//else
		{

		}
	}

	CSocketListener listner(7777);
	listner.ListenStart();

	/////////////////////////////////
	// accept
	while (true)
	{
		bool ret = 
			listner.Accept([pIOCP](SOCKET sock, sockaddr_in addr)
		{
			const char *ip = inet_ntoa(addr.sin_addr);
			Log.Add(L"client connected: %S\n", ip);

			auto* sockObj = new CAsyncSocket(sock);
			sockObj->ConnectToIOCP(*pIOCP);

			sockObj->AsyncRecv();
		});

		if (!ret)
			break;
	}

	listner.Close();
}

