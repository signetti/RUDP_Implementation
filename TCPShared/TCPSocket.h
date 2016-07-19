#pragma once

#include "Socket.h"
#include "Socket.h"

/**
*   TCP socket for communication with other TCP sockets
*/
class TCPSocket : public Socket
{
public:
	/**
	*   Construct a TCP socket with no connection
	*   @exception SocketException thrown if unable to create TCP socket
	*/
	TCPSocket();

	/**
	*   Construct a TCP socket with a connection to the given foreign address
	*   and port
	*   @param foreignAddress foreign address (IP address or name)
	*   @param foreignPort foreign port
	*   @exception SocketException thrown if unable to create TCP socket
	*/
	bool Connect(const std::string& remoteAddress, uint16_t remotePort) override;

	/**
	*   Write the given buffer to this socket.  Call connect() before
	*   calling send()
	*   @param buffer buffer to be written
	*   @param bufferLen number of bytes from buffer to be written
	*   @exception SocketException thrown if unable to send data
	*/
	bool Send(const void *buffer, uint32_t bufferSize) override;

	/**
	*   Read into the given buffer up to bufferLen bytes data from this
	*   socket.  Call connect() before calling recv()
	*   @param buffer buffer to receive the data
	*   @param bufferLen maximum number of bytes to read into buffer
	*   @return number of bytes read, 0 for EOF, and -1 for error
	*   @exception SocketException thrown if unable to receive data
	*/
	uint32_t Receive(void *buffer, uint32_t bufferSize) override;

private:
	// Access for TCPServerSocket::accept() connection creation
	friend class TCPServerSocket;
	TCPSocket(SOCKET newConnSD);
};


/**
*   TCP socket class for servers
*/
class TCPServerSocket : private TCPSocket, public IServerSocket
{
private:
	std::vector<std::shared_ptr<TCPSocket>> mClients;

public:
	/**
	*   Construct a TCP socket for use with a server, accepting connections
	*   on the specified port on any interface
	*   @param localPort local port of server socket, a value of zero will
	*                   give a system-assigned unused port
	*   @param queueLen maximum queue length for outstanding
	*                   connection requests (default 5)
	*   @exception SocketException thrown if unable to create TCP server socket
	*/
	explicit TCPServerSocket(uint16_t listenPort, uint32_t queueLen = 5);
	~TCPServerSocket() override;
	/**
	*   Construct a TCP socket for use with a server, accepting connections
	*   on the specified port on the interface specified by the given address
	*   @param localAddress local interface (address) of server socket
	*   @param localPort local port of server socket
	*   @param queueLen maximum queue length for outstanding
	*                   connection requests (default 5)
	*   @exception SocketException thrown if unable to create TCP server socket
	
	TCPServerSocket(const std::string &localAddress, unsigned short localPort, int queueLen = 5); //throw(SocketException);
	*/

	/**
	*   Blocks until a new connection is established on this socket or error
	*   @return new connection socket
	*   @exception SocketException thrown if attempt to accept a new connection fails
	*/
	TCPSocket *Accept() override;

	uint16_t GetListeningPort() override;

	using TCPSocket::Send;
	using TCPSocket::Receive;

private:
	void Listen(uint32_t queueLen);
};
