/*
*   C++ sockets on Unix and Windows
*   Copyright (C) 2002
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "stdafx.h"
#include "Socket.h"

#pragma warning(disable : 4996)

#ifdef WIN32
#include <winsock.h>         // For socket(), connect(), send(), and recv()
typedef int socklen_t;
typedef char raw_type;       // Type used for raw data on this platform
#else
#include <sys/types.h>       // For data types
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
#include <netinet/in.h>      // For sockaddr_in
typedef void raw_type;       // Type used for raw data on this platform
#endif

#include <errno.h>             // For errno

#include "RPacket.h"

using namespace std;

// SocketException Code

SocketException::SocketException(const string &message, bool inclSysMsg) //throw()
	: userMessage(message)
{
	if (inclSysMsg)
	{
		userMessage.append(": ");
#ifdef WIN32
		char error[100];
		strerror_s(error, errno);
		userMessage.append(error);
#else
		userMessage.append(strerror(errno));
#endif
	}
}

SocketException::~SocketException() //throw()
{}

const char *SocketException::what() const //throw()
{
	return userMessage.c_str();
}

// Function to fill in address structure given an address and port

/* NOT TRUSTWORTHY...
*/
#define __USE_NEW true

#if __USE_NEW
static void fillAddr(const string &address, unsigned short port,
	sockaddr_in &addr)
{
	memset(&addr, 0, sizeof(addr));  // Zero out address structure
	addr.sin_family = AF_INET;       // Internet address

	hostent *host;  // Resolve name
	if ((host = gethostbyname(address.c_str())) == NULL)
	{
		// strerror() will not work for gethostbyname() and hstrerror() 
		// is supposedly obsolete
		throw SocketException("Failed to resolve name (gethostbyname())");
	}
	addr.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);

	addr.sin_port = htons(port);     // Assign port in network byte order
}
#else
static void fillAddr(const string &address, unsigned short port, sockaddr_in &addr)
{
	struct addrinfo hints;
	struct addrinfo * info;
	int result;

	// Get Address Information from the given IP and Port
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	// Resolve the server address and port
	char portBuf[20];
	_itoa_s(port, portBuf, sizeof(portBuf), 10);
	result = getaddrinfo(address.c_str(), portBuf, &hints, &info);
	if (result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", result);
		// strerror() will not work for gethostbyname() and hstrerror() 
		// is supposedly obsolete
		throw SocketException("Failed to resolve name (getaddrinfo())");
	}

	// TODO: See if this actually works...struct sockaddr *sa = ...;
	sockaddr * infoaddr = info->ai_addr;
	if (infoaddr->sa_family == AF_INET)
	{
		addr = *((struct sockaddr_in *) infoaddr);
	}
	else
	{
		throw SocketException("Failed to cast to sockaddr_in (getaddrinfo())");
	}
}
#endif

// Socket Code

Socket::Socket(int type, int protocol) //throw(SocketException)
{
	// Initialize WSA
	WSAManager::StartUp();

	// Make a new socket
	if ((sockDesc = socket(PF_INET, type, protocol)) < 0)
	{
		throw SocketException("Socket creation failed (socket())", true);
	}
}

Socket::Socket(SOCKET sockDesc)
{
	this->sockDesc = sockDesc;
}

Socket::~Socket()
{
	Close();
}

void Socket::Close()
{
#ifdef WIN32
	::closesocket(sockDesc);
#else
	::close(sockDesc);
#endif
	sockDesc = SOCKET(INVALID_SOCKET);
}

bool Socket::IsOpen()
{
	return sockDesc != INVALID_SOCKET && IsConnected();
}

// http://stackoverflow.com/questions/851654/how-can-i-check-is-a-socket-is-still-open
bool Socket::IsConnected()
{
	return true;
	/*char buf;
	int err = recvfrom(sockDesc, &buf, 1, MSG_PEEK, NULL, NULL);
	if (err == SOCKET_ERROR)
	{
		err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK)
		{
			return false;
		}
	}
	return true;*/
}

string Socket::getLocalAddress() //throw(SocketException)
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getsockname(sockDesc, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0)
	{
		throw SocketException("Fetch of local address failed (getsockname())", true);
	}

	return inet_ntoa(addr.sin_addr);
}

unsigned short Socket::getLocalPort() //throw(SocketException)
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getsockname(sockDesc, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0)
	{
		throw SocketException("Fetch of local port failed (getsockname())", true);
	}
	return ntohs(addr.sin_port);
}

void Socket::setLocalPort(unsigned short localPort) //throw(SocketException)
{
	// Bind the socket to its port
	sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(localPort);

	int result = ::bind(sockDesc, (sockaddr *)&localAddr, sizeof(sockaddr_in));
	if (result < 0)
	{
		throw SocketException("Set of local port failed (bind())", true);
	}
}

void Socket::setLocalAddressAndPort(const string &localAddress,
	unsigned short localPort) //throw(SocketException)
{
	// Get the address of the requested host
	sockaddr_in localAddr;
	fillAddr(localAddress, localPort, localAddr);

	if (::bind(sockDesc, (sockaddr *)&localAddr, sizeof(sockaddr_in)) < 0)
	{
		throw SocketException("Set of local address and port failed (bind())", true);
	}
}

void Socket::cleanUp() //throw(SocketException)
{
#ifdef WIN32
	if (WSACleanup() != 0)
	{
		throw SocketException("WSACleanup() failed");
	}
#endif
}

unsigned short Socket::resolveService(const string &service,
	const string &protocol)
{
	struct servent *serv;        /* Structure containing service information */

	if ((serv = getservbyname(service.c_str(), protocol.c_str())) == NULL)
		return (unsigned short)atoi(service.c_str());  /* Service is port number */
	else
		return ntohs(serv->s_port);    /* Found port (network byte order) by name */
}

Socket::Socket(const Socket & sock) : sockDesc(sock.sockDesc) {}

void Socket::operator=(const Socket & sock)
{
	if (this != &sock)
	{
		sockDesc = sock.sockDesc;
	}
}

// CommunicatingSocket Code

CommunicatingSocket::CommunicatingSocket(int type, int protocol) //throw(SocketException)
	: Socket(type, protocol) {}

CommunicatingSocket::CommunicatingSocket(SOCKET newConnSD) : Socket(newConnSD)
{}

void CommunicatingSocket::connect(const string &foreignAddress,
	unsigned short foreignPort) //throw(SocketException)
{
	// Get the address of the requested host
	sockaddr_in destAddr;
	fillAddr(foreignAddress, foreignPort, destAddr);

	// Try to connect to the given port
	if (::connect(sockDesc, (sockaddr *)&destAddr, sizeof(destAddr)) < 0)
	{
		throw SocketException("Connect failed (connect())", true);
	}
}

void CommunicatingSocket::send(const void *buffer, int bufferLen)
//throw(SocketException)
{
	if (::send(sockDesc, (raw_type *)buffer, bufferLen, 0) < 0)
	{
		throw SocketException("Send failed (send())", true);
	}
}

int CommunicatingSocket::recv(void *buffer, int bufferLen)
//throw(SocketException)
{
	int rtn;
	if ((rtn = ::recv(sockDesc, (raw_type *)buffer, bufferLen, 0)) < 0)
	{
		throw SocketException("Received failed (recv())", true);
	}

	return rtn;
}

string CommunicatingSocket::getForeignAddress()
//throw(SocketException)
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getpeername(sockDesc, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0)
	{
		throw SocketException("Fetch of foreign address failed (getpeername())", true);
	}
	return inet_ntoa(addr.sin_addr);
}

unsigned short CommunicatingSocket::getForeignPort() //throw(SocketException)
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getpeername(sockDesc, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0)
	{
		throw SocketException("Fetch of foreign port failed (getpeername())", true);
	}
	return ntohs(addr.sin_port);
}

// TCPSocket Code

TCPSocket::TCPSocket() //throw(SocketException)
	: CommunicatingSocket(SOCK_STREAM, IPPROTO_TCP)
{}

TCPSocket::TCPSocket(const string &foreignAddress, unsigned short foreignPort) //throw(SocketException)
	: CommunicatingSocket(SOCK_STREAM, IPPROTO_TCP)
{
	connect(foreignAddress, foreignPort);
}

TCPSocket::TCPSocket(SOCKET newConnSD) : CommunicatingSocket(newConnSD)
{}

// TCPServerSocket Code

TCPServerSocket::TCPServerSocket(unsigned short localPort, int queueLen) //throw(SocketException)
	: Socket(SOCK_STREAM, IPPROTO_TCP)
{
	setLocalPort(localPort);
	setListen(queueLen);
}

TCPServerSocket::TCPServerSocket(const string &localAddress, unsigned short localPort, int queueLen)	//throw(SocketException)
	: Socket(SOCK_STREAM, IPPROTO_TCP)
{
	setLocalAddressAndPort(localAddress, localPort);
	setListen(queueLen);
}

TCPSocket *TCPServerSocket::accept() //throw(SocketException)
{
	SOCKET newConnSD;
	if ((newConnSD = ::accept(sockDesc, NULL, 0)) < 0)
	{
		throw SocketException("Accept failed (accept())", true);
	}

	return new TCPSocket(newConnSD);
}

void TCPServerSocket::setListen(int queueLen) //throw(SocketException)
{
	if (listen(sockDesc, queueLen) < 0)
	{
		throw SocketException("Set listening socket failed (listen())", true);
	}
}

// UDPSocket Code

UDPSocket::UDPSocket() //throw(SocketException)
	: CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP)
{
	setBroadcast();
}

UDPSocket::UDPSocket(unsigned short localPort)  //throw(SocketException)
	: CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP)
{
	setLocalPort(localPort);
	setBroadcast();
}

UDPSocket::UDPSocket(const string &localAddress, unsigned short localPort) //throw(SocketException)
	: CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP)
{
	setLocalAddressAndPort(localAddress, localPort);
	setBroadcast();
}

void UDPSocket::setBroadcast()
{
	// If this fails, we'll hear about it when we try to send.  This will allow 
	// system that cannot broadcast to continue if they don't plan to broadcast
	int broadcastPermission = 1;
	setsockopt(sockDesc, SOL_SOCKET, SO_BROADCAST,
		(raw_type *)&broadcastPermission, sizeof(broadcastPermission));
}

void UDPSocket::disconnect() //throw(SocketException)
{
	sockaddr_in nullAddr;
	memset(&nullAddr, 0, sizeof(nullAddr));
	nullAddr.sin_family = AF_UNSPEC;

	// Try to disconnect
	if (::connect(sockDesc, (sockaddr *)&nullAddr, sizeof(nullAddr)) < 0)
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

void UDPSocket::sendTo(void *buffer, int bufferLen,
	const string &foreignAddress, unsigned short foreignPort)
	//throw(SocketException)
{
	sockaddr_in destAddr;
	fillAddr(foreignAddress, foreignPort, destAddr);

	// Write out the whole buffer as a single message.
	int bytesSent;

	bytesSent = sendto(sockDesc, static_cast<raw_type *>(buffer), bufferLen, 0, (sockaddr *)&destAddr, sizeof(destAddr));
	if (bytesSent != bufferLen)
	{
		std::string message("Send failed (sendto()) with error ");
		int error = WSAGetLastError();
		char digits[10];
		_itoa_s(error, digits, 10);
		message.append(digits);

		throw SocketException(message.c_str(), true);
	}
}

int UDPSocket::recvFrom(void *buffer, int bufferLen, string &sourceAddress,	unsigned short &sourcePort, int timeOutMS) //throw(SocketException)
{
	sockaddr_in clntAddr;
	socklen_t addrLen = sizeof(clntAddr);
	int bytesReceived;
	int state;
	
	// Use Select to place a delay
	if (timeOutMS >= 0)
	{
		// Setup timeval variable
		timeval timeout;
		timeout.tv_sec = (timeOutMS / 1000);
		timeout.tv_usec = (timeOutMS % 1000) * 1000;

		// Setup fd_set structure
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(sockDesc, &fds);

		// Begin Select (delays for response)
		 state = select(0, &fds, 0, 0, &timeout);

		// Select's return state:
		switch (state)
		{
		case 0:		return 0;															//   0: Timed out
		case -1:	throw SocketException("Select failed (sendto())", true); break;		//  -1: Error occurred
		default:	break;																// > 0: Ready to be read
		}
	}

	bytesReceived = recvfrom(sockDesc, (raw_type *)buffer, bufferLen, 0, (sockaddr *)&clntAddr, (socklen_t *)&addrLen);

	if (bytesReceived < 0)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK && error != WSAEMSGSIZE)
		{
			std::string message("Send failed (sendto()) with error ");
			char digits[10];
			_itoa_s(error, digits, 10);
			message.append(digits);

			throw SocketException(message.c_str(), true);
		}
		else return 0;
	}
	sourceAddress = inet_ntoa(clntAddr.sin_addr);
	sourcePort = ntohs(clntAddr.sin_port);

	return bytesReceived;
}

void UDPSocket::setMulticastTTL(unsigned char multicastTTL) //throw(SocketException)
{
	if (setsockopt(sockDesc, IPPROTO_IP, IP_MULTICAST_TTL,
		(raw_type *)&multicastTTL, sizeof(multicastTTL)) < 0)
	{
		throw SocketException("Multicast TTL set failed (setsockopt())", true);
	}
}

void UDPSocket::joinGroup(const string &multicastGroup) //throw(SocketException)
{
	struct ip_mreq multicastRequest;

	multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastGroup.c_str());
	multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(sockDesc, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(raw_type *)&multicastRequest,
		sizeof(multicastRequest)) < 0)
	{
		throw SocketException("Multicast group join failed (setsockopt())", true);
	}
}

void UDPSocket::leaveGroup(const string &multicastGroup) //throw(SocketException)
{
	struct ip_mreq multicastRequest;

	multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastGroup.c_str());
	multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(sockDesc, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(raw_type *)&multicastRequest,
		sizeof(multicastRequest)) < 0)
	{
		throw SocketException("Multicast group leave failed (setsockopt())", true);
	}
}

void RUDPSocket::sendTo(void * buffer, int bufferLen, const string & foreignAddress, unsigned short foreignPort)
{
	UDPSocket::sendTo(buffer, bufferLen, foreignAddress, foreignPort);
}

int RUDPSocket::recvFrom(void * buffer, int bufferLen, string & sourceAddress, unsigned short & sourcePort, int timeOut)
{
	int bytesReceived = UDPSocket::recvFrom(buffer, bufferLen, sourceAddress, sourcePort, timeOut);

	if (bytesReceived > 0)
	{

	}
	
	return bytesReceived;
}
