#include "stdafx.h"

SOCKET TCPClient::GetServerSocket(struct addrinfo * info)
{
	struct addrinfo* ptr;
	SOCKET serverSocket = INVALID_SOCKET;
	int result;

	// Attempt to connect to an address until one succeeds
	for (ptr = info; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		serverSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (serverSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			return INVALID_SOCKET;
		}

		// Connect to server.
		result = connect(serverSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (result == SOCKET_ERROR)
		{
			closesocket(serverSocket);
			continue;
		}
		break;
	}

	if (serverSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
	}

	return serverSocket;
}

SOCKET TCPClient::InitializeClient(char * ip, char * port)
{
	struct addrinfo hints;
	struct addrinfo * info;

	// Get Address Information from the given IP and Port
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int result;

	// Resolve the server address and port
	result = getaddrinfo(ip, port, &hints, &info);
	if (result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", result);
		return false;
	}

	SOCKET serverSocket = GetServerSocket(info);
	freeaddrinfo(info);

	return serverSocket;
}

TCPStream TCPClient::ConnectToServer(char * ip, char * port)
{
	WSAManager::StartUp();
	return TCPStream(InitializeClient(ip, port));
}
