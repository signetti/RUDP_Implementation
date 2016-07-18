#include "stdafx.h"
#include "WSAManager.h"

// Create instance of WSAManager to start up WSA before main
const WSAManager WSAManager::_instance;

#ifdef WIN32
WSAManager::WSAManager() : WinSockAppData()
{
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
}
#else
WSAManager::WSAManager() {}
#endif

WSAManager::~WSAManager()
{
	// Clean-up WinSock API
#ifdef WIN32
	WSACleanup();
#endif
}

void WSAManager::StartUp() 
{
	// This calling function is to simply assert that the WSAManager is being initialized.
	// Without calling this or one of the other static functions, the static instance will not be compiled in,
	// and therefore will not have its constructor called to make the WSAStartUp() call.
}
