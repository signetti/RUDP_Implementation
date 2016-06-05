#pragma once
#include <winsock2.h>
#include <string>
class TCPStream
{
protected:
	SOCKET mSocket;
public:
	explicit TCPStream(SOCKET socket);
	virtual ~TCPStream();

	// Copy Semantics
	TCPStream(const TCPStream& other);
	virtual TCPStream& operator=(const TCPStream& rhs);

	// Move Semantics
	TCPStream(TCPStream&& other);
	virtual TCPStream& operator=(TCPStream&& rhs);

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

