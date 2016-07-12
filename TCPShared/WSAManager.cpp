#include "stdafx.h"
#include "WSAManager.h"

// Create instance of WSAManager to start up WSA before main
const WSAManager WSAManager::_instance;

WSAManager::WSAManager() : WinSockAppData()
{
#ifdef WIN32
	int result;

	// Request WinSock v2.2
	result = WSAStartup(MAKEWORD(2, 2), &WinSockAppData);
	if (result != 0)
	{  // Load WinSock DLL
		//throw SocketException("Unable to load WinSock DLL");

		std::stringstream message;
		message << "WSAStartup failed with error: " << WSAGetLastError() << '(' << result << ")\n";
		throw std::exception(message.str().c_str());
	}
#endif
}

WSAManager::~WSAManager()
{
	// Clean-up WinSock API
#ifdef WIN32
	WSACleanup();
#endif
}

void WSAManager::StartUp() {}
