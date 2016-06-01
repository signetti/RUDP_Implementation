#pragma once
#include <winsock2.h>

class TCPStream_;

class TCPClient
{
private:
	static SOCKET InitializeClient(char * ip, char * port);
	static SOCKET GetServerSocket(struct addrinfo * info);
public:
	TCPClient() = delete;
	~TCPClient() = delete;

	static TCPStream_ ConnectToServer(char * ip, char * port);
};

