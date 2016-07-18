#pragma once
#include "Socket.h"
#include <vector>
#include <memory>
/**
*   UDP socket class
*/
class UDPSocket : public Socket
{
protected:
	std::string mRemoteAddress;
	uint16_t mRemotePort;
	uint32_t mMaxTimeout;
	bool bIsConnectionOriented;

public:

	/**
	*   Construct a UDP socket
	*   @exception SocketException thrown if unable to create UDP socket
	*/
	explicit UDPSocket(uint32_t maxTimeoutMS = 0U);

	/**
	*   Construct a UDP socket with the given local port
	*   @param localPort local port
	*   @exception SocketException thrown if unable to create UDP socket
	*/
	explicit UDPSocket(uint16_t localPort, uint32_t maxTimeoutMS = 0U);

	/**
	*   Construct a UDP socket with the given local port and address
	*   @param localAddress local address
	*   @param localPort local port
	*   @exception SocketException thrown if unable to create UDP socket
	UDPSocket(const std::string &localAddress, uint16_t localPort);
	*/

	/**/
	void SetMaximumConnectionTimeOut(uint32_t timeoutMS);

	/**
	*   Unset foreign address and port
	*   @return true if disassociation is successful
	*   @exception SocketException thrown if unable to disconnect UDP socket
	void Disconnect(); //throw(SocketException);
	*/

	/**
	*   Send the given buffer as a UDP datagram to the
	*   specified address/port
	*   @param buffer buffer to be written
	*   @param bufferLen number of bytes to write
	*   @param foreignAddress address (IP address or name) to send to
	*   @param foreignPort port number to send to
	*   @return true if send is successful
	*   @exception SocketException thrown if unable to send datagram
	*/
	virtual bool SendTo(const void* buffer, uint32_t bufferSize, const std::string& remoteAddress, uint16_t remotePort);

	/**
	*   Read read up to bufferLen bytes data from this socket.  The given buffer
	*   is where the data will be placed
	*   @param buffer buffer to receive data
	*   @param bufferLen maximum number of bytes to receive
	*   @param sourceAddress address of datagram source
	*   @param sourcePort port of data source
	*   @return number of bytes received and -1 for error
	*   @exception SocketException thrown if unable to receive datagram
	*/
	virtual bool ReceiveFrom(void* OutBuffer, uint32_t& InOutBufferSize, std::string& OutRemoteAddress, uint16_t& OutRemotePort, uint32_t maxTimeoutMS = -1);


	/**/
	virtual bool Connect(const std::string &remoteAddress, uint16_t remotePort) override;
	/**
	*   Send the given buffer as a UDP datagram to the
	*   specified address/port
	*   @param buffer buffer to be written
	*   @param bufferLen number of bytes to write
	*   @param foreignAddress address (IP address or name) to send to
	*   @param foreignPort port number to send to
	*   @return true if send is successful
	*   @exception SocketException thrown if unable to send datagram
	*/
	virtual bool Send(const void *buffer, uint32_t bufferSize) override;

	/**
	*   Read read up to bufferLen bytes data from this socket.  The given buffer
	*   is where the data will be placed
	*   @param buffer buffer to receive data
	*   @param bufferLen maximum number of bytes to receive
	*   @param sourceAddress address of datagram source
	*   @param sourcePort port of data source
	*   @return number of bytes received and -1 for error
	*   @exception SocketException thrown if unable to receive datagram
	*/
	virtual uint32_t Receive(void *buffer, uint32_t bufferSize) override;

	/**/
	std::string GetRemoteAddress() override;

	/**/
	uint16_t GetRemotePort() override;

	/**
	*   Set the multicast TTL
	*   @param multicastTTL multicast TTL
	*   @exception SocketException thrown if unable to set TTL
	void setMulticastTTL(unsigned char multicastTTL);
	*/

	/**
	*   Join the specified multicast group
	*   @param multicastGroup multicast group address to join
	*   @exception SocketException thrown if unable to join group
	void joinGroup(const std::string &multicastGroup);
	*/

	/**
	*   Leave the specified multicast group
	*   @param multicastGroup multicast group address to leave
	*   @exception SocketException thrown if unable to leave group
	void leaveGroup(const std::string &multicastGroup);
	*/

protected:
	void SetBroadcast();
};

class UDPServerSocket : private UDPSocket, public IServerSocket
{
	std::vector<std::shared_ptr<UDPSocket>> mClients;
public:
	explicit UDPServerSocket(uint16_t listenPort, uint32_t maxConnectionTimeout = 1000U);

	UDPSocket* Accept() override;
	uint16_t GetListeningPort() override;
};