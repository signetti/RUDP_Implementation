#pragma once

#include <winsock2.h>
#include <string>            // For string
#include <exception>         // For exception class

#include "SocketGlobal.h"
#include <memory>

/**
*   Base class representing basic communication endpoint
*/
class Socket
{
public:
	/**
	*   Close and deallocate this socket
	*/
	~Socket();
	virtual void Close();
	bool IsOpen();

	/**
	*   Get the local address
	*   @return local address of socket
	*   @exception SocketException thrown if fetch fails
	std::string GetLocalAddress(); //throw(SocketException);
	*/

	/**
	*   Set the local port to the specified port and the local address
	*   to the specified address.  If you omit the port, a random port
	*   will be selected.
	*   @param localAddress local address
	*   @param localPort local port
	*   @exception SocketException thrown if setting local port or address fails
	void setLocalAddressAndPort(const std::string &localAddress,
		unsigned short localPort = 0); //throw(SocketException);
	*/

	/**
	*   Get the local port
	*   @return local port of socket
	*   @exception SocketException thrown if fetch fails
	*/
	uint16_t GetLocalPort();

	/**
	*   Set the local port to the specified port and the local address
	*   to any interface
	*   @param localPort local port
	*   @exception SocketException thrown if setting local port fails
	*/
	void SetLocalPort(uint16_t port);

	/**
	*   Get the foreign address.  Call connect() before calling recv()
	*   @return foreign address
	*   @exception SocketException thrown if unable to fetch foreign address
	*/
	virtual std::string GetRemoteAddress();

	/**
	*   Get the foreign port.  Call connect() before calling recv()
	*   @return foreign port
	*   @exception SocketException thrown if unable to fetch foreign port
	*/
	virtual uint16_t GetRemotePort();

	/**/
	virtual bool Connect(const std::string& remoteAddress, uint16_t remotePort) = 0;
	
	/**
	*   Write the given buffer to this socket.  Call connect() before
	*   calling send()
	*   @param buffer buffer to be written
	*   @param bufferLen number of bytes from buffer to be written
	*   @exception SocketException thrown if unable to send data
	*/
	virtual bool Send(const void *buffer, uint32_t bufferSize) = 0;

	/**
	*   Read into the given buffer up to bufferLen bytes data from this
	*   socket.  Call connect() before calling recv()
	*   @param buffer buffer to receive the data
	*   @param bufferLen maximum number of bytes to read into buffer
	*   @return number of bytes read, 0 for EOF, and -1 for error
	*   @exception SocketException thrown if unable to receive data
	*/
	virtual uint32_t Receive(void *buffer, uint32_t bufferSize) = 0;

	/**
	*   Resolve the specified service for the specified protocol to the
	*   corresponding port number in host byte order
	*   @param service service to resolve (e.g., "http")
	*   @param protocol protocol of service to resolve.  Default is "tcp".
	*/
	static uint16_t ResolveService(const std::string& service, const std::string& protocol = "tcp");

	static void FillAddr(const std::string& address, uint16_t port, sockaddr_in &OutAddr);


	// Must be exposed to be used by RUDPStream
	Socket(const Socket &sock);
	virtual void operator=(const Socket& sock);

private:
	// Prevent the user from trying to use value semantics on this object

protected:
	SOCKET mSocket;              // Socket descriptor
	Socket(int type, int protocol);
	explicit Socket(SOCKET mSocket);
};

class IServerSocket
{
	virtual Socket* Accept() = 0;
	virtual uint16_t GetListeningPort() = 0;
};