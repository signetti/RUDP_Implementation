#pragma once
#include <winsock2.h>
#include <chrono>

#include "Socket.h"

class RUDPStream;
class UDPSocket;

class RUDPClient
{
public:
	// RUDP Client should not be instantiated
	RUDPClient() = delete;
	~RUDPClient() = delete;

	/**
	*	Create a TCPStream that connects to the server of a given IP and Port number.
	*	@param	ip		Server host name or IP address.
	*	@param	port	Server port number.
	*	@return returns a TCPStream with an open socket connecting to the server. Returns an invalid stream if connection fails.
	*/
	static RUDPStream ConnectToServer(const char * ip, unsigned short port, unsigned short clientPort, uint32_t maxConnectionTimeOut);
};

