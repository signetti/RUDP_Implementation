#pragma once

#include <string>
#include <winsock2.h>

#include "SocketGlobal.h"
#include <memory>

/*	C++ base class for Winsock sockets	*/
class Socket
{
protected:
	// The socket descriptor
	SOCKET mSocket;

public:
	/**	Socket destructor. This base class destructor handles closing the socket so children of this class do not have to.	*/
	virtual ~Socket();
	/**	Closes the socket (close() or closesocket() equivalent)	*/
	virtual void Close();
	/**	Checks if this socket is open for connection
	*	@return	true if the socket is open, false otherwise
	*/
	bool IsOpen();

	/**	Get the local address of the socket
	*   @return							local address of the socket
	*   @exception	SocketException		thrown if fetch fails
	*/
	std::string GetLocalAddress();
	

	/**	Get the local port of the socket
	*   @return						local port of socket
	*   @exception	SocketException thrown if fetch fails
	*/
	uint16_t GetLocalPort();

	/**	Sets and binds this socket to the specified port (bind() equivalent)
	*   @param		localPort			local port to set
	*   @exception	SocketException		thrown if setting local port fails
	*/
	void SetLocalPort(uint16_t port);

	/**	Get the remote address for this socket connection
	*   @return							the remote address for this socket communication
	*   @exception	SocketException		thrown if unable to fetch remote address
	*/
	virtual std::string GetRemoteAddress();

	/**	Get the remote port for this socket connection
	*   @return						the remote port for this socket communication
	*   @exception SocketException	thrown if unable to fetch remote port
	*/
	virtual uint16_t GetRemotePort();

	/*	Establishes a connection between the remote client and this client. Varies depending on the type of socket.*/
	virtual bool Connect(const std::string& remoteAddress, uint16_t remotePort) = 0;
	
	/*	Sends the buffer to the remote client. Varies depending on the type of socket.	*/
	virtual bool Send(const void *buffer, uint32_t bufferSize) = 0;

	/*	Receives a message from the remote client and passes it into the buffer. Varies depending on the type of socket.	*/
	virtual uint32_t Receive(void *buffer, uint32_t bufferSize) = 0;

	/**	Resolve the specified service for the specified protocol to the corresponding port number in host byte order
	*   @param service service to resolve (e.g., "http")
	*   @param protocol protocol of service to resolve.  Default is "tcp".
	*/
	static uint16_t ResolveService(const std::string& service, const std::string& protocol = "tcp");

	/**	Given an address and port, construct the struct sockaddr_in with those values.
	*	@param	address		the address to assign
	*	@param	port		the port to assign
	*	@param	OutSockAddr	the struct sockaddr_t to assign the values to
	*/
	static void AssignSockAddr(const std::string& address, uint16_t port, sockaddr_in &OutSockAddr);

protected:
	// The Socket created with a given address family type and protocol (socket() equivalent)
	Socket(int type, int protocol);
	// The socket created by simply assigning the socket to this class
	explicit Socket(SOCKET socket);
};

/*	Interface class for how a server socket should behave	*/
class IServerSocket
{
public:
	/*	Virtual destructor for memory management */
	virtual ~IServerSocket() {}

	/*	Accept an incoming request to connect to this server socket.  Varies depending on the type of socket. */
	virtual Socket* Accept() = 0;
	/*	Gets the port number used to listen for connection. Varies depending on the type of socket.	*/
	virtual uint16_t GetListeningPort() = 0;
};
