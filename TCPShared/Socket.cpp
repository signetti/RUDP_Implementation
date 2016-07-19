#include "stdafx.h"
#include "Socket.h"

#include "SocketGlobal.h"
#include "SocketException.h"
#include "RPacket.h"

Socket::Socket(int type, int protocol)
{
	// Initialize WSA
	WSAManager::StartUp();

	// Make a new socket
	if ((mSocket = socket(PF_INET, type, protocol)) < 0)
	{
		WSAManager::StoreLastErrorCode();
		throw SocketException("Socket creation failed (socket())", true);
	}
}

Socket::Socket(SOCKET mSocket) : mSocket(mSocket) 
{
	// Initialize WSA
	WSAManager::StartUp();
}

Socket::~Socket()
{
	Close();
}

void Socket::Close()
{
	if (mSocket != INVALID_SOCKET)
	{
#ifdef WIN32
		closesocket(mSocket);
		WSAManager::StoreLastErrorCode();
#else
		close(sockDesc);
#endif
	}
	mSocket = SOCKET(INVALID_SOCKET);
}

bool Socket::IsOpen()
{
	return mSocket != INVALID_SOCKET;
}

/*
std::string Socket::GetLocalAddress() //throw(SocketException)
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getsockname(sockDesc, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0)
	{
		throw SocketException("Fetch of local address failed (getsockname())", true);
	}

	return inet_ntoa(addr.sin_addr);
}

void Socket::setLocalAddressAndPort(const std::string &localAddress,
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
*/

unsigned short Socket::GetLocalPort()
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getsockname(mSocket, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0)
	{
		WSAManager::StoreLastErrorCode();
		throw SocketException("Fetch of local port failed (getsockname())", true);
	}
	return ntohs(addr.sin_port);
}

void Socket::SetLocalPort(uint16_t port)
{
	// Bind the socket to its port
	sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(port);

	int result = bind(mSocket, (sockaddr *)&localAddr, sizeof(sockaddr_in));
	WSAManager::StoreLastErrorCode();

	if (result < 0)
	{
		throw SocketException("Set of local port failed (bind())", true);
	}
}

uint16_t Socket::ResolveService(const std::string& service, const std::string &protocol)
{
	struct servent *serv;        /* Structure containing service information */

	serv = getservbyname(service.c_str(), protocol.c_str());
	WSAManager::StoreLastErrorCode();

	if (serv == NULL)
		return (unsigned short)atoi(service.c_str());  /* Service is port number */
	else
		return ntohs(serv->s_port);    /* Found port (network byte order) by name */
}

Socket::Socket(const Socket & sock) : mSocket(sock.mSocket) {}

void Socket::operator=(const Socket & sock)
{
	if (this != &sock)
	{
		mSocket = sock.mSocket;
	}
}


std::string Socket::GetRemoteAddress()
{
	sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	int result = getpeername(mSocket, (sockaddr *)&addr, &addr_len);
	WSAManager::StoreLastErrorCode();

	if (result < 0)
	{
		throw SocketException("Fetch of foreign address failed (getpeername())", true);
	}
	return inet_ntoa(addr.sin_addr);
}

uint16_t Socket::GetRemotePort()
{
	sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	int result = getpeername(mSocket, (sockaddr *)&addr, &addr_len);
	WSAManager::StoreLastErrorCode();

	if (result < 0)
	{
		WSAManager::StoreLastErrorCode();
		throw SocketException("Fetch of foreign port failed (getpeername())", true);
	}
	return ntohs(addr.sin_port);
}


// Function to fill in address structure given an address and port

/* NOT TRUSTWORTHY...
*/
#define __USE_NEW true

#if __USE_NEW
void Socket::FillAddr(const std::string& address, uint16_t port, sockaddr_in &OutAddr)
{
	memset(&OutAddr, 0, sizeof(OutAddr));  // Zero out address structure
	OutAddr.sin_family = AF_INET;       // Internet address

	hostent *host;  // Resolve name

	host = gethostbyname(address.c_str());
	WSAManager::StoreLastErrorCode();

	if (host == NULL)
	{
		// strerror() will not work for gethostbyname() and hstrerror() 
		// is supposedly obsolete
		throw SocketException("Failed to resolve name (gethostbyname())");
	}
	OutAddr.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);

	OutAddr.sin_port = htons(port);     // Assign port in network byte order
}
#else
#include "Logger.h"
void Socket::FillAddr(const std::string& address, uint16_t port, sockaddr_in &addr)
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
		Logger::PrintF(__FILE__, "getaddrinfo failed with error: %d\n", result);
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