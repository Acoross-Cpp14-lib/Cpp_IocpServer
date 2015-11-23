#ifndef _ACOROSS_ASYNCSOCKET_H
#define _ACOROSS_ASYNCSOCKET_H

#pragma once

#include "Utility.h"
#include "Network.h"
#include "IOCP.h"

namespace Acoross {
	namespace Network {
		namespace Async {

			using Acoross::Async::CIOCP;
			using Acoross::Async::CIOObject;
			using Acoross::Network::CSocket;
			using Acoross::Network::CPacketReceiver;
			
			class CPacketHandler
			{
			public:
				bool operator()(const char* pBuf, size_t nLen)
				{
					char buf[100]{ 0, };
					strncpy_s(buf, 100, pBuf, nLen);
					Log.Add("get: %s\n", buf);

					return true;
				}
			};

			/*
				@ CIOObject 를 상속받아 만든, CSocket 에 대한 wrapper
				@ 기능: IOCP 를 이용해 OnIOCallback 처리
					OnIOCallback 에서 -> CSocket 의 ProcessReceived 처리
			*/
			class CAsyncSocket
				: public CIOObject
			{
			public:
				NO_COPY(CAsyncSocket);

				CAsyncSocket(SOCKET sock)
					: m_RecvSocket(sock, CPacketReceiver(CPacketHandler()))
				{
					memset(&m_RecvOverlapped, 0, sizeof(OVERLAPPED));
				}
				virtual ~CAsyncSocket() = default;

				template <size_t N>
				bool ConnectToIOCP(CIOCP<N>& iocp)
				{
					return iocp.ConnectDeviceToIOCP((HANDLE)m_RecvSocket.GetSocket(), (ULONG_PTR)this);
				}

				void AsyncRecv()
				{
					m_RecvSocket.RequestRecv(&m_RecvOverlapped);
				}

			public:
				// virtual
				virtual bool OnIOCallback(DWORD dwTransferred, LPOVERLAPPED lpOverlapped) override
				{
					if (&m_RecvOverlapped == lpOverlapped)
					{
						return m_RecvSocket.ProcessReceived(dwTransferred, lpOverlapped);
					}
					return false;
				}

			private:
				CSocket<1024, CPacketReceiver> m_RecvSocket;
				WSAOVERLAPPED m_RecvOverlapped;
			};	//CAsyncSocket
		}//Async
	}//Network
}//Acoross

#endif //_ACOROSS_ASYNCSOCKET_H