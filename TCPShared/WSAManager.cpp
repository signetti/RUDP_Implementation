#include "stdafx.h"
#include "WSAManager.h"

const WSAManager WSAManager::_instance;// = nullptr;

WSAManager::WSAManager() : WinSockAppData()
{
	int result;

	// Initialize WinSock
	result = WSAStartup(MAKEWORD(2, 2), &WinSockAppData);
	if (result != 0)
	{
		std::stringstream message;
		message << "WSAStartup failed with error: " << WSAGetLastError() << '(' << result << ")\n";
		throw std::exception(message.str().c_str());
	}
}

WSAManager::~WSAManager()
{
	// Clean-up WinSock API
	WSACleanup();
}

void WSAManager::StartUp() {}
