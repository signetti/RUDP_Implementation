#pragma once

#include "Socket.h"
#include "Socket.h"

/*	C++ class representation for a TCP Socket	*/
class TCPSocket : public Socket
{
public:
	/**	Construct a TCP socket with no connection (must call Connect() first)
	*   @exception	SocketException		thrown if unable to create TCP socket
	*/
	TCPSocket();

	/**	Establishes a connection between the remote client and this client (connect() equivalent)
	*	@param	remoteAddress			the remote client's address
	*	@param	remotePort				the remote client's port
	*	@exception	SocketException		thrown if unable to establish TCP connection
	*/
	bool Connect(const std::string& remoteAddress, uint16_t remotePort) override;

	/**	Sends the buffer to the remote client (send() equivalent)
	*   @param	buffer					the buffer to send
	*   @param	bufferSize				the size of the buffer given
	*   @exception SocketException	thrown if unable to send data
	*/
	bool Send(const void *buffer, uint32_t bufferSize) override;

	/**	Receives a message from the remote client and passes it into the buffer (recv() equivalent)
	*   @param	buffer					the buffer for the received message to be stored
	*   @param	bufferSize				the size of the buffer given
	*   @exception SocketException		thrown if unable to send data
	*/
	uint32_t Receive(void *buffer, uint32_t bufferSize) override;

protected:
	// Grants access for TCPServerSocket::Accept() connection creation
	friend class TCPServerSocket;
	// TCPSocket creation using a socket defined for TCP connection
	TCPSocket(SOCKET socket);
};


/*	TCP server socket class	*/
class TCPServerSocket : protected TCPSocket, public IServerSocket
{
private:
	// List of clients accepted by this server
	std::vector<std::shared_ptr<TCPSocket>> mClients;

public:
	/**	Construct a TCP Server Socket, listening on the given port.
	*   @param		localPort			the local port of this server socket. A value of zero will give a system-assigned unused port
	*   @param		queueLen			the maximum queue length for outstanding connection requests (default 5)
	*   @exception	SocketException		thrown if unable to create TCP server socket
	*/
	explicit TCPServerSocket(uint16_t listenPort, uint32_t queueLen = 5U);

	/**	Blocks until a new connection is established on this socket (or an exception is thrown)
	*   @return							a new and open TCPSocket instance
	*   @exception	SocketException		thrown if attempt to accept a new connection fails
	*/
	TCPSocket *Accept() override;

	/**	Retrieves the listening port number that this server is utilizing
	*	@return		the listening port number
	*/
	uint16_t GetListeningPort() override;

private:
	// Given the maximum queue length, set the port to listen with that queue size
	void Listen(uint32_t queueLen);
};
