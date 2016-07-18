#pragma once
#include <winsock2.h>

class WSAManager
{
private:
#ifdef WIN32
	static const WSAManager _instance;
	WSADATA WinSockAppData;
#endif

	WSAManager();
	~WSAManager();
public:
	static void StartUp();
};

