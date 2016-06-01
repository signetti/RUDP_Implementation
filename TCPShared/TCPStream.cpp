#include "stdafx.h"

TCPStream_::TCPStream_(SOCKET socket) : mSocket(socket) {}

TCPStream_::TCPStream_(const TCPStream_ & other) : mSocket(other.mSocket) {}

TCPStream_ & TCPStream_::operator=(const TCPStream_ & rhs)
{
	if (this != &rhs)
	{
		mSocket = rhs.mSocket;
	}
	return *this;
}

TCPStream_::TCPStream_(TCPStream_ && other) : mSocket(std::move(other.mSocket)) {}

TCPStream_ & TCPStream_::operator=(TCPStream_ && rhs)
{
	if (this != &rhs)
	{
		mSocket = std::move(rhs.mSocket);
		rhs.mSocket = INVALID_SOCKET;
	}
	return *this;
}

TCPStream_::~TCPStream_()
{
	Close();
}

int TCPStream_::Write(const char * data, int sizeOfData)
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

int TCPStream_::Write(const char * message)
{
	return Write(message, (int)strlen(message) + 1);
}

int TCPStream_::Write(const std::string& message)
{
	return Write(message.c_str(), static_cast<int>(message.length()) + 1);
}


int TCPStream_::Read(char * OutBuffer, int sizeOfBuffer)
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

bool TCPStream_::Shutdown(int how)
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

void TCPStream_::Close()
{
	// Close Socket
	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
	}
}

bool TCPStream_::IsOpen()
{
	return (mSocket != INVALID_SOCKET);
}
