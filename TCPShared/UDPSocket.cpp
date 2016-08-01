#include "stdafx.h"
#include "SocketGlobal.h"
#include "UDPSocket.h"
#include "SocketException.h"

void UDPSocket::SetBroadcast()
{
	// If this fails, we'll hear about it when we try to send.  This will allow 
	// system that cannot broadcast to continue if they don't plan to broadcast
	int broadcastPermission = 1;
	setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST,
		(raw_type *)&broadcastPermission, sizeof(broadcastPermission));
	WSAManager::StoreLastErrorCode();

	Logger::SetLoggerState(__FILE__, false, true, BasicColor::CYAN);
}

UDPSocket::UDPSocket(uint32_t maxTimeoutMS) : Socket(SOCK_DGRAM, IPPROTO_UDP)
	, mMaxTimeout(maxTimeoutMS), mRemoteAddress(), mRemotePort(), bIsConnectionOriented(false)
{
	SetBroadcast();
}

UDPSocket::UDPSocket(uint16_t localPort, uint32_t maxTimeoutMS) : Socket(SOCK_DGRAM, IPPROTO_UDP)
	, mMaxTimeout(maxTimeoutMS), mRemoteAddress(), mRemotePort(), bIsConnectionOriented(false)
{
	SetLocalPort(localPort);
	SetBroadcast();
}

void UDPSocket::SetMaximumConnectionTimeOut(uint32_t timeoutMS)
{
	mMaxTimeout = timeoutMS;
}

bool UDPSocket::SendTo(const void * buffer, uint32_t bufferSize, const std::string & remoteAddress, uint16_t remotePort)
{
	sockaddr_in destAddr;
	uint32_t bytesSent;
	AssignSockAddr(remoteAddress, remotePort, destAddr);

	bytesSent = sendto(mSocket, reinterpret_cast<const raw_type *>(buffer), bufferSize, 0, (sockaddr *)&destAddr, sizeof(destAddr));
	WSAManager::StoreLastErrorCode();

	Logger::PrintF(__FILE__, "Sent \"%s\" to %s:%d.\n", reinterpret_cast<const char *>(buffer), remoteAddress.c_str(), remotePort);

	if (bytesSent != bufferSize)
	{
		throw SocketException("Send failed (sendto()) with error ", true, true);
	}

	return (bytesSent != 0);
}

bool UDPSocket::ReceiveFrom(void * OutBuffer, uint32_t& InOutBufferSize, std::string & OutRemoteAddress, uint16_t & OutRemotePort, uint32_t maxTimeoutMS)
{
	sockaddr_in clntAddr;
	socklen_t addrLen = sizeof(clntAddr);
	int bytesReceived;
	int state;
	
	// Use Select() to create a delay
	if (maxTimeoutMS > 0)
	{
		// Setup timeval variable
		timeval timeout;
		timeout.tv_sec = (maxTimeoutMS / 1000);
		timeout.tv_usec = (maxTimeoutMS % 1000) * 1000;

		// Setup fd_set structure
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(mSocket, &fds);

		// Begin Select (delays for response)
		 state = select(0, &fds, 0, 0, &timeout);
		 WSAManager::StoreLastErrorCode();

		// Select's return state:
		switch (state)
		{
		case 0:		return false;														//   0: Timed out
		case -1:	throw SocketException("Select failed (select())", true); break;		//  -1: Error occurred
		default:	break;																// > 0: Ready to be read
		}
	}

	bytesReceived = recvfrom(mSocket, reinterpret_cast<raw_type *>(OutBuffer), InOutBufferSize, 0, (sockaddr *)&clntAddr, (socklen_t *)&addrLen);
	WSAManager::StoreLastErrorCode();

	if (bytesReceived < 0)
	{
		int error = WSAManager::GetLastErrorCode();
		if (error != WSAEWOULDBLOCK && error != WSAEMSGSIZE)
		{
			throw SocketException("Receive failed (recvfrom())", true, true);
		}
		else return false;
	}
	else if (bytesReceived != 0)
	{
		// Reassign the newly retrieved address and port
		OutRemoteAddress = inet_ntoa(clntAddr.sin_addr);
		OutRemotePort = ntohs(clntAddr.sin_port);
		WSAManager::StoreLastErrorCode();
	}

	Logger::PrintF(__FILE__, "Received \"%s\" from %s:%d.\n", reinterpret_cast<const char *>(OutBuffer), OutRemoteAddress.c_str(), OutRemotePort);

	// Return Bytes Received
	InOutBufferSize = bytesReceived;
	return true;
}

bool UDPSocket::Connect(const std::string & serverAddress, uint16_t serverPort)
{

	// Send to server, to complete connection
	uint32_t bytesReceived;
	std::string remoteAddress;
	uint16_t remotePort;

	char buffer[2];
	uint32_t bufferSize;
	do
	{
		// Send Message
		bufferSize = 2;
		SendTo("S", bufferSize, serverAddress, serverPort);

		// Receive from Client
		bytesReceived = ReceiveFrom(buffer, bufferSize, remoteAddress, remotePort, mMaxTimeout);
		if (remoteAddress != serverAddress)
		{	// Received message that is not from the given remote address:port, ignore...
			bytesReceived = 0;
		}
	} while (bytesReceived == 0);

	mRemoteAddress = remoteAddress;
	mRemotePort = remotePort;
	bIsConnectionOriented = true;

	return true;
}

bool UDPSocket::Send(const void *buffer, uint32_t bufferSize)
{
	return SendTo(buffer, bufferSize, mRemoteAddress, mRemotePort);
}

uint32_t UDPSocket::Receive(void *buffer, uint32_t bufferSize)
{
	// Simple UDP Receive, do not write-over current remote client
	std::string farseAddress;
	uint16_t farsePort;

	bool isSuccess = ReceiveFrom(buffer, bufferSize, farseAddress, farsePort, mMaxTimeout);

	if (bIsConnectionOriented)
	{
		if (farsePort != mRemotePort || farseAddress != mRemoteAddress)
		{	// Received message that is not from the given remote address:port, ignore...
			memset(buffer, 0, bufferSize);	// For safety purposes
		}
	}
	else
	{	// Set the remote address and port received from
		mRemoteAddress = farseAddress;
		mRemotePort = farsePort;
	}

	return (isSuccess) ? bufferSize : 0;
}

std::string UDPSocket::GetRemoteAddress()
{
	return mRemoteAddress;
}

uint16_t UDPSocket::GetRemotePort()
{
	return mRemotePort;
}



UDPServerSocket::UDPServerSocket(uint16_t listenPort, uint32_t maxTimeoutMS) : UDPSocket(listenPort, maxTimeoutMS), mAvailablePort(listenPort + 1), mClients(){}

UDPSocket * UDPServerSocket::Accept()
{
	std::shared_ptr<UDPSocket> client = std::make_shared<UDPSocket>(mAvailablePort, mMaxTimeout);

	// Simple UDP Receive, do not write-over current remote client
	std::string remoteAddress;
	uint16_t remotePort;
	char buffer[1200];
	uint32_t bufferSize = sizeof(buffer);
	bool isSuccess;
	
	do
	{
		isSuccess = ReceiveFrom(buffer, bufferSize, remoteAddress, remotePort, mMaxTimeout);
	} while (!isSuccess);
	
	// Successfully retrieved a UDP message, make the remote sender the connection to establish
	client->mRemoteAddress = remoteAddress;
	client->mRemotePort = remotePort;
	client->bIsConnectionOriented = true;
	client->SendTo("C", 2, remoteAddress, remotePort);

	mClients.push_back(client);
	++mAvailablePort;
	return client.get();
}

uint16_t UDPServerSocket::GetListeningPort()
{
	return GetLocalPort();
}