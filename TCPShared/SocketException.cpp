#include "stdafx.h"
#include "SocketException.h"

SocketException::SocketException(const std::string &message, bool isIncludingWSAErrorMessage, bool isIncludingSystemMessage)
	: mErrorMessage(), mWSAError((uint16_t)WSAGetLastError())
{
	{	// Create the Error Message that this resulted in...
		std::stringstream message_stream;
		message_stream << message;

		if (isIncludingWSAErrorMessage)
		{	// Print WSA Error Message
			auto& wsa_error = WSAManager::GetLastError();

			message_stream << "\n\nWSA_Error: #" << wsa_error.Code() << " - " << wsa_error.CodeName() << '\n';
			message_stream << wsa_error.QuickDescription() << '\n';
			message_stream << wsa_error.Description();
		}

		if(isIncludingSystemMessage)
		{	// Print System Error Message
			message_stream << "\n\n";
#ifdef WIN32
			char buffer[100];
			strerror_s(buffer, errno);
			message_stream << buffer;
#else
			message_stream << strerror(errno);
#endif
		}

		// Store the Error message
		mErrorMessage = message_stream.str();
	}

	// Print the Error Message
	Logger::PrintError(__FILE__, mErrorMessage);
}

SocketException::~SocketException() {}

const char *SocketException::what() const
{
	return mErrorMessage.c_str();
}