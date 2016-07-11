#pragma once
#include <string>

#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

class SocketUtils
{
public:
	SocketUtils();
	~SocketUtils();

	/*
	// Function to fill in address structure given an address and port
	static void fillAddr(const std::string &address, unsigned short port, sockaddr_in &OutAddr)
	{
		memset(&OutAddr, 0, sizeof(OutAddr));  // Zero out address structure
		OutAddr.sin_family = AF_INET;       // Internet address

		hostent *host;  // Resolve name
		if ((host = gethostbyname(address.c_str())) == NULL)
		{
			// strerror() will not work for gethostbyname() and hstrerror() 
			// is supposedly obsolete
			throw std::exception("Failed to resolve name (gethostbyname())");
		}
		OutAddr.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);

		OutAddr.sin_port = htons(port);     // Assign port in network byte order
	}*/
};

