#pragma once

#ifndef _IOCP_H_
#define _IOCP_H_

#include <Windows.h>
#include <array>

#include "Utility.h"

namespace Acoross{
namespace Async
{
	struct CIOThreadControlBlock;

	using IOThreadFunc = std::function<bool(CIOThreadControlBlock*)>;
	struct CIOThreadControlBlock
	{
		int nID;
		HANDLE hThread;
		HANDLE hIOCP;
		IOThreadFunc threadFunc;

		bool bStop{ false };
	};

	static DWORD __stdcall IOThread(LPVOID arg)
	{
		if (arg == nullptr)
			return -1;

		auto* pData = (CIOThreadControlBlock*)arg;
		if (pData->threadFunc)
			(pData->threadFunc)(pData);

		return 0;
	}

	class CIOObject
	{
	public:
		virtual bool OnIOCallback() = 0;
	};

	template<size_t count>
	class CIOCP
	{
	public:
		NO_COPY(CIOCP);
		CIOCP() = default;

		bool CreateIOCP()
		{
			static_assert(count <= 16, "IOThread count is too much");

			hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, count);
			if (hIOCP == NULL)
			{
				Log.Add(L"CreateIOCP failed: %d\n", GetLastError());
				return false;
			}
			return true;
		}

		bool ConnectDeviceToIOCP(HANDLE hDevice, ULONG_PTR completionKey)
		{
			if (CreateIoCompletionPort(hDevice, hIOCP, completionKey, 0) == NULL)
			{
				Log.Add(L"ConnectDeviceToIOCP failed: %d\n", GetLastError());
				return false;
			}
			return true;
		}

		bool BeginIOThreadPool(IOThreadFunc func)
		{
			bool bSuccess = true;
			int id = 0;
			for (CIOThreadControlBlock& threadBlock : m_hThreadArr)
			{
				threadBlock.nID = id;
				threadBlock.hIOCP = hIOCP;
				threadBlock.threadFunc = func;	// don't move! copy!
				threadBlock.bStop = false;
				
				threadBlock.hThread = CreateThread(NULL, 0, IOThread, &threadBlock, CREATE_SUSPENDED, (LPDWORD)&id);
				if (threadBlock.hThread == NULL)
				{
					Log.Add(L"CreateThread[%d] is failed: %d\n", id, GetLastError());
					bSuccess = false;
					break;
				}

				++id;
			}

			if (bSuccess)
			{
				for (CIOThreadControlBlock& threadBlock : m_hThreadArr)
				{
					ResumeThread(threadBlock.hThread);
				}
				return true;
			}
			else
			{
				return false;
			}
		}

		void Clear()
		{
			CloseHandle(hIOCP);

			for (CIOThreadControlBlock& threadBlock : m_hThreadArr)
			{
				if (threadBlock.hThread != NULL)
				{
					threadBlock.bStop = true;
					CloseHandle(threadBlock.hThread);
					threadBlock.hThread = NULL;
				}
			}
		}

	private:
		HANDLE hIOCP{ NULL };
		std::array<CIOThreadControlBlock, count> m_hThreadArr;
	};
}//Async
}//Acoross

#endif //_IOCP_H_