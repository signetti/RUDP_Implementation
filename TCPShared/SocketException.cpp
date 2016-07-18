#include "stdafx.h"
#include "SocketException.h"

// SocketException Code
SocketException::SocketException(const std::string &message, bool inclSysMsg) //throw()
	: userMessage(message)
{
	if (inclSysMsg)
	{
		userMessage.append(": ");
#ifdef WIN32
		char error[100];
		strerror_s(error, errno);
		userMessage.append(error);
#else
		userMessage.append(strerror(errno));
#endif
	}
}

SocketException::~SocketException() //throw()
{}

const char *SocketException::what() const //throw()
{
	return userMessage.c_str();
}