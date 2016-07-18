#pragma once
#include "Socket.h"
//
///**
//*   Socket which is able to connect, send, and receive
//*/
//class CommunicatingSocket : public Socket
//{
//public:
//	/**
//	*   Establish a socket connection with the given foreign
//	*   address and port
//	*   @param foreignAddress foreign address (IP address or name)
//	*   @param foreignPort foreign port
//	*   @exception SocketException thrown if unable to establish connection
//	*/
//	void connect(const std::string &foreignAddress, unsigned short foreignPort);
//	//	throw(SocketException);
//
//	/**
//	*   Write the given buffer to this socket.  Call connect() before
//	*   calling send()
//	*   @param buffer buffer to be written
//	*   @param bufferLen number of bytes from buffer to be written
//	*   @exception SocketException thrown if unable to send data
//	*/
//	void send(const void *buffer, int bufferLen); //throw(SocketException);
//
//	/**
//	*   Read into the given buffer up to bufferLen bytes data from this
//	*   socket.  Call connect() before calling recv()
//	*   @param buffer buffer to receive the data
//	*   @param bufferLen maximum number of bytes to read into buffer
//	*   @return number of bytes read, 0 for EOF, and -1 for error
//	*   @exception SocketException thrown if unable to receive data
//	*/
//	int recv(void *buffer, int bufferLen); //throw(SocketException);
//
//	/**
//	*   Get the foreign address.  Call connect() before calling recv()
//	*   @return foreign address
//	*   @exception SocketException thrown if unable to fetch foreign address
//	*/
//	std::string getForeignAddress(); //throw(SocketException);
//
//	/**
//	*   Get the foreign port.  Call connect() before calling recv()
//	*   @return foreign port
//	*   @exception SocketException thrown if unable to fetch foreign port
//	*/
//	unsigned short getForeignPort(); //throw(SocketException);
//
//protected:
//	CommunicatingSocket(int type, int protocol); //throw(SocketException);
//	CommunicatingSocket(SOCKET newConnSD);
//};
