#pragma once
#include <winsock2.h>

class TCPStream;

class TCPClient
{
private:
	static SOCKET InitializeClient(char * ip, char * port);
	static SOCKET GetServerSocket(struct addrinfo * info);
public:
	TCPClient() = delete;
	~TCPClient() = delete;

	static TCPStream ConnectToServer(char * ip, char * port);
};

