#pragma once

#ifndef _SERVER_H_
#define _SERVER_H_

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <utility>
#include <list>
#include <mutex>

#include "Utility.h"

namespace Acoross{
namespace Network
{
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

	// CServer
	// role : 
		// listen socket 을 유지한다.
		// client 의 connect 요청에 대해 accept 처리한다.
		// Sty 타입의 Client Socket 을 생성하고 own 한다.
		// Client Socket 에 대해 OnConnected 를 호출해준다.
	template <class STy>
	class CServer
	{
	public:
		using SocketCreatorFunction = std::unique_ptr<STy>(SOCKET, sockaddr_in);
		using SocketCreator = std::function<SocketCreatorFunction>;

		// SocketCreator 이 template 인자라서 sockCrt 는 universal reference 임.
		CServer(SocketCreator&& sockCrt)
			: m_SocketCreator(sockCrt)
		{
		}

		// listen socket open, and listen start
		bool ListenStart()
		{
			GuardLock Lock(m_ServerLock);

			if (!m_SocketCreator)
			{
				Log.Add(L"no SocketCreator\n");
				return false;
			}

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
			service.sin_port = htons(27016);

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
		bool AcceptLoop()
		{
			while (true)
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
				
				{
					GuardLock Lock(m_ServerLock);
					m_ClientSockets.emplace_back(m_SocketCreator(ClientSocket, cliaddr));
				}
			}

			Close();
			return true;
		}

		bool Close()
		{
			GuardLock Lock(m_ServerLock);

			if (listenSocket != INVALID_SOCKET)
			{
				if (closesocket(listenSocket) == SOCKET_ERROR)
				{
					Log.Add(L"closesocket function failed with error %d\n", WSAGetLastError());
				}
				listenSocket = INVALID_SOCKET;
			}

			Log.Add(L"listen socket closed.\n");

			for (auto& sock : m_ClientSockets)
			{
				sock->Close();
			}
			m_ClientSockets.clear();
			
			return true;
		}

	private:
		SOCKET listenSocket{ NULL };	
		const SocketCreator m_SocketCreator;

		std::mutex						m_ServerLock;
		std::list<std::shared_ptr<STy>>	m_ClientSockets;	// CServer "owns" Sockets.
	};//CServer
}//Network
}//Acoross

#endif //_SERVER_H_