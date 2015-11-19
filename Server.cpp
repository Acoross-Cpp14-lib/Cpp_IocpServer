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
	CSocketListener listner(7777);
	listner.ListenStart();

	while (true)
	{
		listner.Accept([](SOCKET sock, sockaddr_in addr)
		{
			char ipAddress[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(addr.sin_addr), ipAddress, INET_ADDRSTRLEN);

			const char *ip = inet_ntoa(addr.sin_addr);
			Log.Add(L"client connected: %S\n", ip);

			char buf[] = "hi. close this. bye.\n";
			send(sock, buf, strlen(buf), 0);
			closesocket(sock);
		});
	}

	listner.Close();
}

}//Network
}//Acoross
	

