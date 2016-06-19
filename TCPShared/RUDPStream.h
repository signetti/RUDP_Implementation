#pragma once
#include <winsock2.h>
#include <string>

class RUDPStream
{
//protected:
public:
	//The socket with which the stream is established
	SOCKET mSocket;
	// Address information where to send the packets
	struct sockaddr mToAddress;
	// Time (in milliseconds) to wait before determine a connection is lost
	uint32_t mMaxConnectionTimeOut;

	// Sock address size for packet sending
	static const int32_t SOCK_ADDR_SIZE = sizeof(struct sockaddr_in);
public:
	/**
	*	TCPStream Constructor
	*	@param	socket		The socket with which the stream is established
	*/
	explicit RUDPStream(const SOCKET& socket, const struct sockaddr& toAddress, uint32_t maxConnectionTimeOut = 2000);

	/**
	*	TCPStream Constructor for receiving RUDP Connections only
	*	@param	socket		The socket with which the stream is established
	*/
	explicit RUDPStream(const SOCKET& socket);
	/**
	*	TCPStream Destructor
	*/
	virtual ~RUDPStream();

	/**
	*	Copy semantics for TCPStream
	*	@param	other	The TCPStream being copied to this instance
	*/
	RUDPStream(const RUDPStream& other);
	/**
	*	Copy semantics for TCPStream
	*	@param	rhs		The TCPStream being copied to this instance
	*	@return	returns a self reference
	*/
	virtual RUDPStream& operator=(const RUDPStream& rhs);

	/**
	*	Move semantics for TCPStream
	*	@param	other	The TCPStream being moved to this instance
	*/
	RUDPStream(RUDPStream&& other);
	/**
	*	Move semantics for TCPStream
	*	@param	rhs		The TCPStream being moved to this instance
	*	@return	returns a self reference
	*/
	virtual RUDPStream& operator=(RUDPStream&& rhs);

	/**
	*	Send the message to the receiving end of the stream
	*	@param	message		The message to send to the receiver
	*	@return returns the number of bytes sent. '0' means a drop in connection, and less than '0' is an error.
	*/
	int Send(const char * message);
	/**
	*	Send the message to the receiving end of the stream
	*	@param	message		The message to send to the receiver
	*	@return returns the number of bytes sent. '0' means a drop in connection, and less than '0' is an error.
	*/
	int Send(const std::string& message);
	/**
	*	Send the message to the receiving end of the stream
	*	@param	message		The message to send to the receiver, as raw bytes
	*	@return returns the number of bytes sent. '0' means a drop in connection, and less than '0' is an error.
	*/
	virtual int Send(const char * data, int sizeOfData);

	/**
	*	Wait to receive a message from the end of the stream
	*	@param	OutBuffer		The buffer to write the message to
	*	@param	sizeOfBuffer	The size of the buffer
	*	@return returns the number of bytes written to the OutBuffer. '0' means a drop in connection, and less than '0' is an error.
	*/
	virtual int Receive(char * OutBuffer, int sizeOfBuffer);

	/**
	*	Shuts down a specific functionality of the stream. Simply a wrapper to the function shutdown(SOCKET,int).
	*	@param	how		How to shutdown the socket, use enumeration SHUT_RD and SHUT_WR.
	*	@return returns true if shutdown was succesful, false otherwise.
	*/
	bool Shutdown(int how);

	/**
	*	Closes the socket used in this stream.
	*/
	virtual void Close();

	/**
	*	Checks if the socket is open or valid.
	*	@return returns true if the stream is connected (open and valid), false otherwise.
	*/
	bool IsOpen();
};