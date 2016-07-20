#include "stdafx.h"
#include "Socket.h"

#include "SocketGlobal.h"
#include "SocketException.h"
#include "RPacket.h"

Socket::Socket(int type, int protocol)
{
	if ((mSocket = socket(PF_INET, type, protocol)) < 0)
	{
		WSAManager::StoreLastErrorCode();
		throw SocketException("Socket creation failed (socket())", true, true);
	}
}

Socket::Socket(SOCKET socket) : mSocket(socket) {}

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


std::string Socket::GetLocalAddress()
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getsockname(mSocket, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0)
	{
		throw SocketException("Fetch of local address failed (getsockname())", true, true);
	}

	return inet_ntoa(addr.sin_addr);
}

unsigned short Socket::GetLocalPort()
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getsockname(mSocket, (sockaddr *)&addr, (socklen_t *)&addr_len) < 0)
	{
		WSAManager::StoreLastErrorCode();
		throw SocketException("Fetch of local port failed (getsockname())", true, true);
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
		throw SocketException("Set of local port failed (bind())", true, true);
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

std::string Socket::GetRemoteAddress()
{
	sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	int result = getpeername(mSocket, (sockaddr *)&addr, &addr_len);
	WSAManager::StoreLastErrorCode();

	if (result < 0)
	{
		throw SocketException("Fetch of foreign address failed (getpeername())", true, true);
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
		throw SocketException("Fetch of foreign port failed (getpeername())", true, true);
	}
	return ntohs(addr.sin_port);
}

void Socket::AssignSockAddr(const std::string& address, uint16_t port, sockaddr_in& OutSockAddr)
{
	memset(&OutSockAddr, 0, sizeof(OutSockAddr));  // Zero out address structure
	OutSockAddr.sin_family = AF_INET;       // Internet address

	hostent *host;  // Resolve name

	host = gethostbyname(address.c_str());
	WSAManager::StoreLastErrorCode();

	if (host == NULL)
	{
		// strerror() will not work for gethostbyname() and hstrerror() (is supposedly obsolete)
		throw SocketException("Failed to resolve name (gethostbyname())", true, false);
	}
	OutSockAddr.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);

	OutSockAddr.sin_port = htons(port);     // Assign port in network byte order
}