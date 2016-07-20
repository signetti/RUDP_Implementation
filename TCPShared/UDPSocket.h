#pragma once
#include "Socket.h"
#include <vector>
#include <memory>

/*	C++ class representation for a UDP Socket	*/
class UDPSocket : public Socket
{
protected:
	// The remote address
	std::string mRemoteAddress;
	uint16_t mRemotePort;
	uint32_t mMaxTimeout;
	bool bIsConnectionOriented;

public:
	/**	Construct a UDP socket
	*	@param		maxTimeoutMS		the maximum time given to wait on data from a remote client (in milliseconds)
	*   @exception	SocketException		thrown if unable to create an RUDP socket
	*/
	explicit UDPSocket(uint32_t maxTimeoutMS = 0U);

	/**	Construct a UDP socket binded to the given local port
	*	@param		localPort			the port this socket will bind to
	*	@param		maxTimeoutMS		the maximum time given to wait on data from a remote client (in milliseconds)
	*   @exception	SocketException		thrown if unable to create an RUDP socket
	*/
	explicit UDPSocket(uint16_t localPort, uint32_t maxTimeoutMS = 0U);

	/**	Set the maximum time to wait on data from a remote client
	*	@param		timeoutMS		timeout in milliseconds
	*/
	void SetMaximumConnectionTimeOut(uint32_t timeoutMS);

	/** Send the buffer to the specified remote client (sendto() equivalent)
	*   @param		buffer				the buffer to send
	*   @param		bufferSize			the size of the buffer given
	*   @param		remoteAddress		address (IP address or name) to send to
	*   @param		remotePort			port number to send to
	*   @return							true if send is successful
	*   @exception	SocketException		thrown if unable to send datagram
	*/	
	virtual bool SendTo(const void* buffer, uint32_t bufferSize, const std::string& remoteAddress, uint16_t remotePort);

	/**	Receives a message from the remote client and passes it into the buffer (recvfrom() equivalent).
	*	@note		The remote client's address and port, as well as the size of the message received, is returned through the parameter.
	*   @param		buffer					the buffer for the received message to be stored
	*   @param		InOutBufferSize			insert the size of the buffer given and reassigns it with the new size of the same buffer for the contained message
	*   @param		remoteAddress			assigns the address to the remote client that sent the message
	*   @param		remotePort				assigns the port to the remote client that sent the message
	*	@param		maxTimeoutMS		the maximum time given to wait on data from a remote client (in milliseconds)
	*   @return							true if send is successful
	*   @exception	SocketException		thrown if unable to receive datagram
	*/
	virtual bool ReceiveFrom(void* OutBuffer, uint32_t& InOutBufferSize, std::string& OutRemoteAddress, uint16_t& OutRemotePort, uint32_t maxTimeoutMS = -1);

	/**	Establishes a connection between the remote client and this client (by simply storing the remote client info).
	*	@param	remoteAddress			the remote client's address
	*	@param	remotePort				the remote client's port
	*/
	virtual bool Connect(const std::string &remoteAddress, uint16_t remotePort) override;

	/**	Sends the buffer to the remote client (that is assigned when Connect() is called).
	*   @param	buffer					the buffer to send
	*   @param	bufferSize				the size of the buffer given
	*   @exception SocketException		thrown if unable to send data
	*/
	virtual bool Send(const void *buffer, uint32_t bufferSize) override;

	/**	Receives a message from the remote client (that is assigned when Connect() is called) and passes it into the buffer.
	*	@note	This function sets the remote address and port if no connection is established (through the Connect() function call).
	*   @param	buffer					the buffer for the received message to be stored
	*   @param	bufferSize				the size of the buffer given
	*   @exception SocketException		thrown if unable to send data
	*/
	virtual uint32_t Receive(void *buffer, uint32_t bufferSize) override;

	/**	Get the remote address that this UDP socket has saved for a "connection"
	*   @return	the remote address stored
	*/
	std::string GetRemoteAddress() override;

	/**	Get the remote port that this UDP socket has saved for a "connection"
	*   @return	the remote port stored
	*/
	uint16_t GetRemotePort() override;

protected:
	// Set socket to broadcast
	void SetBroadcast();
};


/*	UDP server socket class	*/
class UDPServerSocket : private UDPSocket, public IServerSocket
{
	// List of clients accepted by this server
	std::vector<std::shared_ptr<UDPSocket>> mClients;
public:
	/**	Construct a UDP Server Socket, listening on the given port.
	*   @param		localPort			the local port of this server socket. A value of zero will give a system-assigned unused port
	*	@param		maxTimeoutMS		the maximum time given to wait on data from a remote client (in milliseconds)
	*   @exception	SocketException		thrown if unable to create TCP server socket
	*/
	explicit UDPServerSocket(uint16_t listenPort, uint32_t maxTimeoutMS = 1000U);

	/**	Blocks until a message is received from a remote client, and establish them for the connection.
	*   @return							a new and open UDPSocket instance
	*   @exception	SocketException		thrown if attempt to accept a new connection fails
	*/
	UDPSocket* Accept() override;

	/**	Retrieves the listening port number that this server is utilizing
	*	@return		the listening port number
	*/
	uint16_t GetListeningPort() override;
};