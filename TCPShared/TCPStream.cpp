#include "stdafx.h"

TCPStream::TCPStream(SOCKET socket) : mSocket(socket) {}

TCPStream::TCPStream(const TCPStream & other) : mSocket(other.mSocket) {}

TCPStream & TCPStream::operator=(const TCPStream & rhs)
{
	if (this != &rhs)
	{
		mSocket = rhs.mSocket;
	}
	return *this;
}

TCPStream::TCPStream(TCPStream && other) : mSocket(std::move(other.mSocket)) {}

TCPStream & TCPStream::operator=(TCPStream && rhs)
{
	if (this != &rhs)
	{
		mSocket = std::move(rhs.mSocket);
		rhs.mSocket = INVALID_SOCKET;
	}
	return *this;
}

TCPStream::~TCPStream()
{
	Close();
}

int TCPStream::Send(const char * data, int sizeOfData)
{
	int result;

	// Send an initial buffer
	result = send(mSocket, data, sizeOfData, 0);
	if (result == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		return -1;
	}

	return result;
}

int TCPStream::Send(const char * message)
{
	return Send(message, (int)strlen(message) + 1);
}

int TCPStream::Send(const std::string& message)
{
	return Send(message.c_str(), static_cast<int>(message.length()) + 1);
}


int TCPStream::Receive(char * OutBuffer, int sizeOfBuffer)
{
	int result;

	// Receive bytes
	result = recv(mSocket, OutBuffer, sizeOfBuffer, 0);
	if (result == SOCKET_ERROR)
	{
		printf("recv failed with error: %d\n", WSAGetLastError());
	}
	return result;
}

bool TCPStream::Shutdown(int how)
{
	int result;

	// shutdown the sending connection
	result = shutdown(mSocket, how);
	if (result == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		return false;
	}
	else return true;
}

void TCPStream::Close()
{
	// Close Socket
	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
	}
}

bool TCPStream::IsOpen()
{
	return (mSocket != INVALID_SOCKET);
}
