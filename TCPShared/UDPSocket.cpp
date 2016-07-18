#include "stdafx.h"
#include "SocketGlobal.h"
#include "UDPSocket.h"
#include "SocketException.h"

// UDPSocket Code

/*
UDPSocket::UDPSocket(const std::string &localAddress, unsigned short localPort) : Socket(SOCK_DGRAM, IPPROTO_UDP)
{
	setLocalAddressAndPort(localAddress, localPort);
	setBroadcast();
}*/

void UDPSocket::SetBroadcast()
{
	// If this fails, we'll hear about it when we try to send.  This will allow 
	// system that cannot broadcast to continue if they don't plan to broadcast
	int broadcastPermission = 1;
	setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST,
		(raw_type *)&broadcastPermission, sizeof(broadcastPermission));
}

/*
void UDPSocket::Disconnect()
{
	sockaddr_in nullAddr;
	memset(&nullAddr, 0, sizeof(nullAddr));
	nullAddr.sin_family = AF_UNSPEC;

	// Try to disconnect
	if (connect(sockDesc, (sockaddr *)&nullAddr, sizeof(nullAddr)) < 0)
	{
#ifdef WIN32
		if (errno != WSAEAFNOSUPPORT)
		{
#else
		if (errno != EAFNOSUPPORT)
		{
#endif
			throw SocketException("Disconnect failed (connect())", true);
		}
	}
}
*/

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

bool UDPSocket::Connect(const std::string & remoteAddress, uint16_t remotePort)
{
	mRemoteAddress = remoteAddress;
	mRemotePort = remotePort;
	bIsConnectionOriented = true;
	return true;
}

void UDPSocket::SetMaximumConnectionTimeOut(uint32_t timeoutMS)
{
	mMaxTimeout = timeoutMS;
}

bool UDPSocket::SendTo(const void * buffer, uint32_t bufferSize, const std::string & remoteAddress, uint16_t remotePort)
{
	sockaddr_in destAddr;
	FillAddr(remoteAddress, remotePort, destAddr);

	// Write out the whole buffer as a single message.
	uint32_t bytesSent;

	bytesSent = sendto(mSocket, reinterpret_cast<const raw_type *>(buffer), bufferSize, 0, (sockaddr *)&destAddr, sizeof(destAddr));
	if (bytesSent != bufferSize)
	{
		std::string message("Send failed (sendto()) with error ");
		int error = WSAGetLastError();
		char digits[10];
		_itoa_s(error, digits, 10);
		message.append(digits);

		throw SocketException(message.c_str(), true);
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

		// Select's return state:
		switch (state)
		{
		case 0:		return false;														//   0: Timed out
		case -1:	throw SocketException("Select failed (sendto())", true); break;		//  -1: Error occurred
		default:	break;																// > 0: Ready to be read
		}
	}

	bytesReceived = recvfrom(mSocket, reinterpret_cast<raw_type *>(OutBuffer), InOutBufferSize, 0, (sockaddr *)&clntAddr, (socklen_t *)&addrLen);

	if (bytesReceived < 0)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK && error != WSAEMSGSIZE)
		{
			std::string message("Receive failed (sendto()) with error ");
			char digits[10];
			_itoa_s(error, digits, 10);
			message.append(digits);

			throw SocketException(message.c_str(), true);
		}
		else return false;
	}
	else if (bytesReceived != 0)
	{
		// Reassign the newly retrieved address and port
		OutRemoteAddress = inet_ntoa(clntAddr.sin_addr);
		OutRemotePort = ntohs(clntAddr.sin_port);
	}

	// Return Bytes Received
	InOutBufferSize = bytesReceived;
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

	uint32_t bytesReceived = ReceiveFrom(buffer, bufferSize, farseAddress, farsePort, mMaxTimeout);

	if (bIsConnectionOriented)
	{
		if (farsePort != mRemotePort || farseAddress != mRemoteAddress)
		{	// Received message that is not from the given remote address:port, ignore...
			memset(buffer, 0, bufferSize);	// For safety purposes
			bytesReceived = 0;
		}
	}
	else
	{	// Set the remote address and port received from
		mRemoteAddress = farseAddress;
		mRemotePort = farsePort;
	}

	return bytesReceived;
}

std::string UDPSocket::GetRemoteAddress()
{
	return mRemoteAddress;
}

uint16_t UDPSocket::GetRemotePort()
{
	return mRemotePort;
}

/*
void UDPSocket::setMulticastTTL(unsigned char multicastTTL) //throw(SocketException)
{
	if (setsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_TTL,
		(raw_type *)&multicastTTL, sizeof(multicastTTL)) < 0)
	{
		throw SocketException("Multicast TTL set failed (setsockopt())", true);
	}
}

void UDPSocket::joinGroup(const std::string &multicastGroup) //throw(SocketException)
{
	struct ip_mreq multicastRequest;

	multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastGroup.c_str());
	multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(mSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(raw_type *)&multicastRequest,
		sizeof(multicastRequest)) < 0)
	{
		throw SocketException("Multicast group join failed (setsockopt())", true);
	}
}

void UDPSocket::leaveGroup(const std::string &multicastGroup) //throw(SocketException)
{
	struct ip_mreq multicastRequest;

	multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastGroup.c_str());
	multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(mSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(raw_type *)&multicastRequest,
		sizeof(multicastRequest)) < 0)
	{
		throw SocketException("Multicast group leave failed (setsockopt())", true);
	}
}
*/


UDPServerSocket::UDPServerSocket(uint16_t listenPort, uint32_t maxConnectionTimeout) : UDPSocket(listenPort, maxConnectionTimeout) {}

UDPSocket * UDPServerSocket::Accept()
{
	std::shared_ptr<UDPSocket> client = std::make_shared<UDPSocket>(mMaxTimeout);

	// Simple UDP Receive, do not write-over current remote client
	std::string remoteAddress;
	uint16_t remotePort;
	char buffer[1200];
	uint32_t bufferSize = sizeof(buffer);
	bool isSuccess = ReceiveFrom(buffer, bufferSize, remoteAddress, remotePort, mMaxTimeout);

	if (isSuccess)
	{	// Successfully retrieved a UDP message, make the remote sender the connection to establish
		client->Connect(remoteAddress, remotePort);
		mClients.push_back(client);
		return client.get();
	}
	else
	{	// No message received, no connection accepted...
		return nullptr;
	}
}
uint16_t UDPServerSocket::GetListeningPort()
{
	return GetLocalPort();
}