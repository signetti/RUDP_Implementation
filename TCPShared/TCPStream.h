#pragma once
#include <winsock2.h>
#include <string>
class TCPStream_
{
protected:
	SOCKET mSocket;
public:
	explicit TCPStream_(SOCKET socket);
	virtual ~TCPStream_();

	// Copy Semantics
	TCPStream_(const TCPStream_& other);
	virtual TCPStream_& operator=(const TCPStream_& rhs);

	// Move Semantics
	TCPStream_(TCPStream_&& other);
	virtual TCPStream_& operator=(TCPStream_&& rhs);

	// Send
	int Write(const char * message);
	int Write(const std::string& message);
	virtual int Write(const char * data, int sizeOfData);

	// Receive
	virtual int Read(char * OutBuffer, int sizeOfBuffer);

	// Shutdown
	bool Shutdown(int how);

	// Close Socket
	virtual void Close();

	// Checks if socket is open
	bool IsOpen();
};

