#pragma once
#include <winsock2.h>
#include <string>

class TCPStream
{
protected:
	//The socket with which the stream is established
	SOCKET mSocket;

public:
	/**
	*	TCPStream Constructor
	*	@param	socket		The socket with which the stream is established
	*/
	explicit TCPStream(SOCKET socket);
	/**
	*	TCPStream Destructor
	*/
	virtual ~TCPStream();

	/**
	*	Copy semantics for TCPStream
	*	@param	other	The TCPStream being copied to this instance
	*/
	TCPStream(const TCPStream& other);
	/**
	*	Copy semantics for TCPStream
	*	@param	rhs		The TCPStream being copied to this instance
	*	@return	returns a self reference
	*/
	virtual TCPStream& operator=(const TCPStream& rhs);

	/**
	*	Move semantics for TCPStream
	*	@param	other	The TCPStream being moved to this instance
	*/
	TCPStream(TCPStream&& other);
	/**
	*	Move semantics for TCPStream
	*	@param	rhs		The TCPStream being moved to this instance
	*	@return	returns a self reference
	*/
	virtual TCPStream& operator=(TCPStream&& rhs);

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

