#pragma once

#ifndef _ACOROSS_NETWORK_LISTNER_H
#define _ACOROSS_NETWORK_LISTNER_H

#include <utility>
#include <functional>

#include "NetworkDefault.h"

#include "Utility.h"

namespace Acoross {
	namespace Network
	{
		using AcceptEventFuncT = void(SOCKET, sockaddr_in);
		using AcceptEventHandler = std::function<AcceptEventFuncT>;

		/* Acceptor
		@ thread: single thread
		@ role: start listen, accept, close
		*/
		class Acceptor
		{
		public:
			NO_COPY(Acceptor);

			// SocketCreator 이 template 인자라서 sockCrt 는 universal reference 임.
			Acceptor(int port) : m_port(port)
			{
			}

			Acceptor(Acceptor&& rhs)
				: m_port(rhs.m_port), listenSocket(rhs.listenSocket)
			{
			}

			// listen socket open, and listen start
			bool ListenStart()
			{
				//--------------------
				// Create listen socket
				listenSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
				if (listenSocket == INVALID_SOCKET)
				{
					Log.Add(L"WSASocket failed with error: %ld\n", WSAGetLastError());
					Close();
					return false;
				}

				//--------------------
				// bind socket
				sockaddr_in service;
				service.sin_family = AF_INET;
				service.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
				service.sin_port = htons(m_port);

				if (bind(listenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
				{
					Log.Add(L"bind function failed with error %d\n", WSAGetLastError());
					Close();
					return false;
				}

				Log.Add(L"bind started.\n");

				//--------------------
				// listen socket
				if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
				{
					Log.Add(L"listen error");
					Close();
					return false;
				}

				Log.Add(L"listen started.\n");

				return true;
			}

			// accept client socket, create client socket and insert to list
			bool Accept(const AcceptEventHandler& AcceptEvent)
			{
				//--------------------
				// accept socket
				SOCKET ClientSocket = INVALID_SOCKET;
				sockaddr_in cliaddr;

				ClientSocket = accept(listenSocket, (SOCKADDR*)&cliaddr, NULL);
				if (ClientSocket == INVALID_SOCKET)
				{
					Log.Add(L"error, accept function error: %d\n", WSAGetLastError());
					Close();
					return false;
				}

				AcceptEvent(ClientSocket, cliaddr);

				return true;
			}

			bool Close()
			{
				if (listenSocket != INVALID_SOCKET)
				{
					if (closesocket(listenSocket) == SOCKET_ERROR)
					{
						Log.Add(L"closesocket function failed with error %d\n", WSAGetLastError());
					}
					listenSocket = INVALID_SOCKET;
				}

				Log.Add(L"listen socket closed.\n");

				return true;
			}

		private:
			SOCKET listenSocket{ NULL };
			const int m_port;
		};
		//CSocketListener
	}
}

#endif //_ACOROSS_NETWORK_LISTNER_H
