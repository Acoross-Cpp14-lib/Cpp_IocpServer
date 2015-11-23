#ifndef _SOCKET_H_
#define _SOCKET_H_

#pragma once

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
	template<size_t N>
	struct CIOBuffer
	{
		const int size{ N };
		int nRemain{ 0 };
		char buffer[N]{ 0, };
	};
	
	/*
		@ ����:
			@ �������� ��Ŷ�� recv
			@ recv �Ϸ�� ��Ŷ��, PacketHandler �̿��� ó��

		@ ��Ŷ ���ۿ� ���� ����: 
			@ �������� (�ִ� 1024 = len + body ����)
			@ len(2)body(n) - len ���� body ���� (�ִ� 1022)
			@ ������ ���� �� ����.
			@ �ִ� ���� �Ѵ� ��Ŷ�� ������ �� ��. �� ��� ���� �������. (���� ��Ŷ�� ����)
			@ ���� �ӽŰ� endian ������ ������ ����.
	*/
	class CPacketReceiver
	{
	public:
		typedef std::function<bool(const char*, size_t)> PacketCmddHandler;

		enum RecvResult
		{
			RECV_CLOSE = -1,	// ���� ������.
			RECV_OVER_MAX_LEN = -2,	// ��Ŷ�� �ʹ� ���.
			RECV_BUF_NULL = -3,		// ��Ŷ ���۰� NULL.
			RECV_PACKET_HANDLE_FAIL = -4,	// ��Ŷ�ڵ鷯���� ��Ŷó�� ����
		};

		CPacketReceiver(PacketCmddHandler& handler)
			: m_PacketCmdHandler(handler)
		{
		}

		CPacketReceiver(PacketCmddHandler&& handler)
			: m_PacketCmdHandler(std::move(handler))
		{
		}

		// return: 
		// < 0: error
		// >= 0: remaining packet data in buffer (should remain until next call)
		template <size_t N>
		int OnRecv(DWORD nRemain, DWORD dwTransferred, char (&pBuf)[N])
		{
			return OnRecv(nRemain, dwTransferred, pBuf, N);
		}

		// return: 
		// < 0: error
		// >= 0: processed data length
		int OnRecv(DWORD nRemain, DWORD dwTransferred, const char* pBuf, size_t N)
		{
			if (dwTransferred == 0)
			{
				return RECV_CLOSE;
			}

			if (pBuf == nullptr)
			{
				return RECV_BUF_NULL;
			}

			unsigned int f = 0; // front
			unsigned int dataLen = dwTransferred + nRemain;
			while (true)
			{
				// read header
				if (f + 2 > dataLen)
				{
					return f;
				}
				
				// len �� length byte (2) �� ������ �����̴�. ( 2 + body len )
				const unsigned short len = pBuf[f + 1] << 8 | pBuf[f + 0];	// little endian
				if (len > N)
				{
					return RECV_OVER_MAX_LEN;
				}

				if (f + len > dataLen)
				{
					return f;	// remaining packet data length.
				}

				// ó���� �� �ִ� ��Ŷ�� �����ϸ�
				if (false == m_PacketCmdHandler(&pBuf[f + 2], len - 2))
				{
					// ��Ŷó�� ����!
					return RECV_PACKET_HANDLE_FAIL;
				}
				
				f += len;
			}
		}

	private:
		PacketCmddHandler m_PacketCmdHandler;
	};

	/*
		@ socket handle �� �̿��� Recv �� RecvCallback �� ó��
		- sync / async ��� ó�� �����ϸ�, async ó���� CIOObject �� wrapper �����ؼ� ó��
	*/
	template <size_t BufferSize,
				class PacketReceiverT>
	class CSocket
	{
	public:
		typedef CIOBuffer<BufferSize> BufferT;
		NO_COPY(CSocket);

		CSocket(SOCKET sock, PacketReceiverT&& packetRecv)
			: socket(sock), pReadBuffer(nullptr), packetReceiver(packetRecv)
		{
			pReadBuffer = new BufferT;
		}
		~CSocket()
		{
			Close();	// ��������� �� �� ������, �Ҹ��ڿ��� �� ���� �ִ�.
			Log.Add(L"CAsyncSocket dtor()\n");
			delete pReadBuffer;
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

		void RequestRecv(LPOVERLAPPED lpOverlapped = nullptr)
		{
			WSABUF wsabuf;

			int nRemain = pReadBuffer->nRemain;
			wsabuf.buf = &pReadBuffer->buffer[nRemain];
			wsabuf.len = pReadBuffer->size - nRemain;
			DWORD dwNumOfByteReceived = 0;
			DWORD flag = 0;

			if (WSARecv(socket, &wsabuf, 1, &dwNumOfByteReceived, &flag, lpOverlapped, NULL) == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				if (WSA_IO_PENDING != err)
				{
					Log.Add(L"WSARecv failed with error: %d\n", err);
					Close();
				}
			}
		}

		bool ProcessReceived(DWORD dwTransferred, LPOVERLAPPED lpOverlapped)
		{
			int ret = packetReceiver.OnRecv(pReadBuffer->nRemain, dwTransferred, pReadBuffer->buffer);
			if (ret < 0)
			{
				Log.Add("packet handle error: %d\n", ret);
				return false;
			}

			if (ret > 0)
			{
				auto& pBuf = pReadBuffer->buffer;

				// 0: success
				if (0 != memmove_s(pBuf, sizeof(pBuf), &pBuf[ret], pReadBuffer->nRemain + dwTransferred - ret))
				{
					Log.Add("memmove error!!! %s, %d\n", TEXT(__FILE__), __LINE__);
					return false;
				}
			}

			pReadBuffer->nRemain += (dwTransferred - ret);
			RequestRecv(lpOverlapped);
			return true;
		}

		SOCKET GetSocket() const
		{
			return socket;
		}

	private:
		SOCKET socket;
		
		BufferT* pReadBuffer{ nullptr };
		//CPacketReceiver packetReceiver;
		PacketReceiverT packetReceiver;
	};
}//Network
}//Acoross

#endif //_SOCKET_H_