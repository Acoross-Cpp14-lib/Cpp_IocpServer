#pragma once

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <utility>
#include <list>
#include <mutex>
#include <memory>

#include "Utility.h"
#include "IOCP.h"

using Acoross::Async::CIOObject;
using Acoross::Async::CIOCP;
using Acoross::Log;

void CALLBACK dummy_overlap_callback(
	IN DWORD dwError,
	IN DWORD cbTransferred,
	IN LPWSAOVERLAPPED lpOverlapped,
	IN DWORD dwFlags
	)
{
	Log.Add(L"dummy callback\n");
}

namespace Acoross {
namespace Network
{
	class CSocket : public CIOObject
	{
	public:
		NO_COPY(CSocket)
			CSocket(SOCKET sock, sockaddr_in addr, std::weak_ptr<CIOCP<8>> iocpWP)
			: clientSocket(sock), cliaddr(addr), IOCPWP(iocpWP)
		{
			char buf[46] = { 0, };
			InetNtop(AF_INET, &cliaddr.sin_addr, buf, 46);
			Log.Add(L"client %S accepted\n", buf);

			_OnConnected();
		}

		~CSocket()
		{	
			Close();
		}

		void Close()
		{
			GuardLock Lock(sockLock);
			if (clientSocket != INVALID_SOCKET)
			{
				closesocket(clientSocket);
				clientSocket = INVALID_SOCKET;
			}

			char buf[46] = { 0, };
			InetNtop(AF_INET, &cliaddr.sin_addr, buf, 46);
			Log.Add(L"client %S closed\n", buf);
		}

		virtual bool OnIOCallback() override
		{
			Log.Add(L"OnIOCallback\n");
			
			return true;
		}

	private:
		void _OnConnected()
		{
			if (auto iocpSP = IOCPWP.lock())
			{
				if (!iocpSP->ConnectDeviceToIOCP((HANDLE)clientSocket, (ULONG_PTR)this))
				{
					closesocket(clientSocket);
					clientSocket = INVALID_SOCKET;
				}
			}
/*
			if (send(clientSocket, "hahaha", 6, 0) == SOCKET_ERROR)
			{
				Log.Add(L"send failed with error: %d\n", WSAGetLastError());
				closesocket(clientSocket);
				clientSocket = INVALID_SOCKET;
			}
*/
			m_wsabuf.buf = new char[1000]{ 0, };
			m_wsabuf.len = 1000;
			DWORD dwNumOfByteReceived = 0;
			DWORD flag= 0;
			if (WSARecv(clientSocket, &m_wsabuf, 1, &dwNumOfByteReceived, &flag, &m_overlap, NULL) == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				if (WSA_IO_PENDING != err)
				{
					Log.Add(L"WSARecv failed with error: %d\n", err);
					closesocket(clientSocket);
					clientSocket = INVALID_SOCKET;
				}
			}
		}

		std::mutex sockLock;	// lock for CSocket
		SOCKET clientSocket{ NULL };
		sockaddr_in cliaddr;
		std::weak_ptr<CIOCP<8>> IOCPWP;
		WSABUF m_wsabuf;
		WSAOVERLAPPED m_overlap;
	};//CSocket
}//Network
}//Acoross

#endif //_SOCKET_H_