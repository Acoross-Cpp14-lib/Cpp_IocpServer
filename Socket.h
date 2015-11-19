#pragma once

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "NetworkDefault.h"

#include <utility>
#include <list>
#include <mutex>
#include <memory>

#include "Utility.h"
#include "IOCP.h"

using Acoross::Async::CIOObject;
using Acoross::Async::CIOCP;
using Acoross::Log;

//void CALLBACK dummy_overlap_callback(
//	IN DWORD dwError,
//	IN DWORD cbTransferred,
//	IN LPWSAOVERLAPPED lpOverlapped,
//	IN DWORD dwFlags
//	)
//{
//	Log.Add(L"dummy callback\n");
//}

namespace Acoross {
namespace Network
{	
	/*CSocket
		@ listen �� ���� ������ ����.
		@ role: ����, �Ҹ�?
				��� accept �� �ܺο��� ������, closesocket �� CSocket ���� ó���Ѵ�.
	*/
	class CSocket
	{
	public:
		NO_COPY(CSocket);

		CSocket(SOCKET sock) 
			: socket(sock)
		{
		}

		~CSocket()
		{
			Close();	// ��������� �� �� ������, �Ҹ��ڿ��� �� ���� �ִ�.
		}

		void Close()
		{
			if (socket != INVALID_SOCKET)
			{
				closesocket(socket);
				socket = INVALID_SOCKET;
			}
		}

	private:
		SOCKET socket;
	};

	// �Ʒ� Ŭ������ ������.
	// �ϴ� ���� ���� ���ܳ��´�.
	class CSocketDeprecated : public CIOObject
	{
	public:
		NO_COPY(CSocketDeprecated);

		CSocketDeprecated(SOCKET sock, sockaddr_in addr, std::weak_ptr<CIOCP<8>> iocpWP)
			: clientSocket(sock), cliaddr(addr), IOCPWP(iocpWP)
		{
			char buf[46] = { 0, };
			InetNtop(AF_INET, &cliaddr.sin_addr, buf, 46);
			Log.Add(L"client %S accepted\n", buf);

			_OnConnected();
		}

		~CSocketDeprecated()
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
			
			Read();

			return true;
		}

		void Read()
		{
			WSABUF wsabuf;
			wsabuf.buf = m_buffer;
			wsabuf.len = BUFLEN;
			DWORD dwNumOfByteReceived = 0;
			DWORD flag = 0;
			if (WSARecv(clientSocket, &wsabuf, 1, &dwNumOfByteReceived, &flag, &m_overlap, NULL) == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				if (WSA_IO_PENDING != err)
				{
					Log.Add(L"WSARecv failed with error: %d\n", err);
					Close();
				}
			}
		}

	private:
		void _OnConnected()
		{
			if (auto iocpSP = IOCPWP.lock())
			{
				if (!iocpSP->ConnectDeviceToIOCP((HANDLE)clientSocket, (ULONG_PTR)this))
				{
					Close();
				}
			}

			Read();
		}

		std::mutex sockLock;	// lock for CSocket
		SOCKET clientSocket{ NULL };
		sockaddr_in cliaddr;

		std::weak_ptr<CIOCP<8>> IOCPWP;
		
		constexpr static int BUFLEN{ 1000 };
		char m_buffer[BUFLEN]{ 0, };
		WSAOVERLAPPED m_overlap;
	};//CSocket
}//Network
}//Acoross

#endif //_SOCKET_H_