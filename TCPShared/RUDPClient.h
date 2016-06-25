#pragma once
#include <winsock2.h>
#include <chrono>

class RUDPStream;

class RUDPClient
{
private:

	static int Connect(const SOCKET& serverSocket, const struct sockaddr& serverAddress, uint32_t maxConnectionTimeOut);
	// Gets the address information and calls GetServerSocket to complete socket creation.
	static RUDPStream GetServerStream(char * ip, char * port, char * clientPort, uint32_t maxConnectionTimeOut);


	// Gets the address information and calls GetServerSocket to complete socket creation.
	static RUDPStream InitializeClient(char * ip, char * port, uint32_t maxConnectionTimeOut);
	// Returns a socket that is connected to server, returns invalid socket if connection fails.
	static RUDPStream GetServerSocket(struct addrinfo * info, uint32_t maxConnectionTimeOut);
public:
	// TCP Client should not be instantiated
	RUDPClient() = delete;
	~RUDPClient() = delete;

	/**
	*	Create a TCPStream that connects to the server of a given IP and Port number.
	*	@param	ip		Server host name or IP address.
	*	@param	port	Server port number.
	*	@return returns a TCPStream with an open socket connecting to the server. Returns an invalid stream if connection fails.
	*/
	static RUDPStream ConnectToServer(char * ip, char * port, char * clientPort, uint32_t maxConnectionTimeOut);
};

