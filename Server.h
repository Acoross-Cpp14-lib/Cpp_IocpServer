#pragma once

#ifndef _ACOROSS_NETWORK_SERVER_H_
#define _ACOROSS_NETWORK_SERVER_H_

#define _WINSOCK_DEPRECATED_NO_WARNINGS

namespace Acoross{
namespace Network
{
	// initialize Winsock
	bool Init();

	void Cleanup();
	
	class CServer
	{
	public:
		void Run();
	};

}//Network
}//Acoross

#endif //_ACOROSS_NETWORK_SERVER_H_