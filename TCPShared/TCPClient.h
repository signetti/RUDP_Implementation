#pragma once
#include <winsock2.h>

class TCPStream;

class TCPClient
{
private:
	// Gets the address information and calls GetServerSocket to complete socket creation.
	static SOCKET InitializeClient(char * ip, char * port);
	// Returns a socket that is connected to server, returns invalid socket if connection fails.
	static SOCKET GetServerSocket(struct addrinfo * info);
public:
	// TCP Client should not be instantiated
	TCPClient() = delete;
	~TCPClient() = delete;

	/**
	*	Create a TCPStream that connects to the server of a given IP and Port number.
	*	@param	ip		Server host name or IP address.
	*	@param	port	Server port number.
	*	@return returns a TCPStream with an open socket connecting to the server. Returns an invalid stream if connection fails.
	*/
	static TCPStream ConnectToServer(char * ip, char * port);
};

