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
		@ listen 을 통해 생성된 소켓.
		@ role: 생성, 소멸?
				비록 accept 는 외부에서 되지만, closesocket 은 CSocket 에서 처리한다.
	*/
	class CAsyncSocket : public CIOObject
	{
	public:
		NO_COPY(CAsyncSocket);

		CAsyncSocket(SOCKET sock) 
			: socket(sock)
		{
			memset(&m_overlap, 0, sizeof(OVERLAPPED));
		}

		virtual ~CAsyncSocket()
		{
			Close();	// 명시적으로 할 수 있으며, 소멸자에서 될 수도 있다.
			Log.Add(L"CAsyncSocket dtor()\n");
		}
		
		void AsyncRecv()
		{
			WSABUF wsabuf;
			wsabuf.buf = m_buffer;
			wsabuf.len = BUFLEN;
			DWORD dwNumOfByteReceived = 0;
			DWORD flag = 0;
			if (WSARecv(socket, &wsabuf, 1, &dwNumOfByteReceived, &flag, &m_overlap, NULL) == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				if (WSA_IO_PENDING != err)
				{
					Log.Add(L"WSARecv failed with error: %d\n", err);
					Close();
				}
			}
		}

		virtual bool OnIOCallback(DWORD dwTransferred, LPOVERLAPPED lpOverlapped) override
		{
			if (false == OnRecvCallback(dwTransferred, lpOverlapped))
			{
				return false;
			}

			return true;
		}
		
		void Close()
		{
			if (socket != INVALID_SOCKET)
			{
				closesocket(socket);
				socket = INVALID_SOCKET;
				Log.Add(L"socket closed.\n");
			}
		}

	private:
		bool OnRecvCallback(DWORD dwTransferred, LPOVERLAPPED lpOverlapped)
		{
			if (dwTransferred == 0)
				return false;

			char prnBuf[100]{ '\0', };
			if (0 == memmove_s(prnBuf, sizeof(prnBuf), m_buffer, dwTransferred)) // 0: success
			{
				Log.Add(prnBuf);
			}

			AsyncRecv();

			return true;
		}

	private:
		SOCKET socket;

		constexpr static int BUFLEN{ 1000 };
		char m_buffer[BUFLEN]{ 0, };
		WSAOVERLAPPED m_overlap;
	};
}//Network
}//Acoross

#endif //_SOCKET_H_