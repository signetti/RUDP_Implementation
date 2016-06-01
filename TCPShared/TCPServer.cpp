#include "stdafx.h"

TCPServer::TCPServer(const std::string& listenPort) : mListenSocket(INVALID_SOCKET), mInfo(NULL), mPort(listenPort)
{
	// Start-Up the WSA Library
	WSAManager::StartUp();
}

TCPServer::~TCPServer()
{
	Close();
}

bool TCPServer::Open()
{
	bool success;

	// Create Listening Socket (if not created)
	success = CreateSocket();
	if (!success) return false;

	// Bind Listening Socket to port
	success = Bind();
	if (!success) return false;

	// Listen in on port
	success = Listen();
	if (!success) return false;
	else return true;
}

bool TCPServer::CreateSocket()
{
	// Check if listening socket is already created
	if (mInfo != NULL && mListenSocket != INVALID_SOCKET)
	{
		return true;
	}

	struct addrinfo hints;
	int result;

	// Check that the address information is not retrieved
	if (mInfo == NULL)
	{
		// Get Address Information from this computer
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		result = getaddrinfo(NULL, mPort.c_str(), &hints, &mInfo);
		if (result != 0)
		{
			printf("getaddrinfo failed with error: %d\n", result);
			mInfo = NULL;
			return false;
		}
	}

	// Create Listening Socket
	mListenSocket = socket(mInfo->ai_family, mInfo->ai_socktype, mInfo->ai_protocol);
	if (mListenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		return false;
	}
	else return true;
}

bool TCPServer::Bind()
{
	int result;

	// Make sure that the listening socket has been created
	if (mInfo == NULL || mListenSocket == INVALID_SOCKET)
	{
		return false;
	}

	// Setup the TCP listening socket
	result = bind(mListenSocket, mInfo->ai_addr, (int)mInfo->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(mListenSocket);
		return false;
	}
	else return true;
}

bool TCPServer::Listen()
{
	int result;

	// Make sure socket is created before this call
	if (mInfo == NULL || mListenSocket == INVALID_SOCKET)
	{
		return false;
	}

	result = listen(mListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(mListenSocket);
		mListenSocket = INVALID_SOCKET;
		return false;
	}
	else return true;
}

TCPStream_ TCPServer::Accept()
{
	// Accept a client socket
	SOCKET clientSocket = accept(mListenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(mListenSocket);
		mListenSocket = INVALID_SOCKET;
	}

	// Create client and store it into vector-list
	TCPStream_ * newClientStream = new TCPStream_(clientSocket);
	mClients.push_back(newClientStream);

	// Return the new client
	return *(newClientStream);
}

void TCPServer::Close()
{
	// Clear address information
	if (mInfo == NULL)
	{
		freeaddrinfo(mInfo);
		mInfo = NULL;
	}

	// Close listening socket
	if (mListenSocket != INVALID_SOCKET)
	{
		closesocket(mListenSocket);
		mListenSocket = INVALID_SOCKET;
	}

	// Clear list of clients
	for (auto& stream : mClients)
	{
		if (stream->IsOpen())
		{
			stream->Close();
			delete stream;
		}
	}
	mClients.clear();
}