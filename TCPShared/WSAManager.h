#pragma once
#include <winsock2.h>

class WSAManager
{
private:
	static const WSAManager _instance;

	WSADATA WinSockAppData;

	WSAManager();
	~WSAManager();
public:
	static void StartUp();
};

