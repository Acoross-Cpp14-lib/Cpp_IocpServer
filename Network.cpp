#include "Network.h"

#include "NetworkDefault.h"

namespace Acoross
{
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
	}//Network
}//Acoross
