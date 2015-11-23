#pragma once

#ifndef _ACOROSS_NETWORK_H_
#define _ACOROSS_NETWORK_H_

#include "Socket.h"
#include "listener.h"

namespace Acoross {
	namespace Network
	{
		// initialize Winsock
		bool Init();

		void Cleanup();
	}//Network
}

#endif //_ACOROSS_NETWORK_H_