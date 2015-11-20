#include "Server.h"

#include "NetworkDefault.h"

#include <utility>
#include <functional>

#include "Utility.h"
#include "listener.h"
#include "Socket.h"

namespace Acoross{
namespace Network
{

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
			if (false == pObject->OnIOCallback(dwTransferred, pOverlap))
			{
				delete pObject;
			}

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

// initialize Winsock
bool Init()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		Log.Add(L"error!");
		WSACleanup();
		return false;
	}
	return true;
}

void Cleanup()
{
	WSACleanup();
}

void CServer::Run()
{
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

	while (true)
	{
		listner.Accept([pIOCP](SOCKET sock, sockaddr_in addr)
		{
			const char *ip = inet_ntoa(addr.sin_addr);
			Log.Add(L"client connected: %S\n", ip);

			auto* sockObj = new CAsyncSocket(sock);
			pIOCP->ConnectDeviceToIOCP((HANDLE)sock, (ULONG_PTR)sockObj);
			sockObj->AsyncRecv();
		});
	}

	listner.Close();
}

}//Network
}//Acoross
	

