/*
*   C++ sockets on Unix and Windows
*   Copyright (C) 2002
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "stdafx.h"
#include "UDPSocket.h"
#include "SocketException.h"
#include <iostream>          // For cout and cerr
#include <cstdlib>           // For atoi()

#include "ConfigReader.h"

const int ECHOMAX = 255;     // Longest string to echo

int server_main(int argc, char *argv[])
{
	if (argc != 2)
	{                  // Test for correct number of parameters
		std::cerr << "Usage: " << argv[0] << " <Server Port>" << std::endl;
		exit(1);
	}

	unsigned short echoServPort = (unsigned short)atoi(argv[1]);     // First arg:  local port

	try
	{
		UDPSocket sock(echoServPort);

		char echoBuffer[ECHOMAX];         // Buffer for echo string
		uint32_t recvMsgSize;                  // Size of received message
		std::string sourceAddress;             // Address of datagram source
		unsigned short sourcePort;        // Port of datagram source
		for (;;)
		{  // Run forever
		   // Block until receive message from a client
			recvMsgSize = ECHOMAX;

			sock.ReceiveFrom(echoBuffer, recvMsgSize, sourceAddress,	sourcePort);

			std::cout << "Received packet from " << sourceAddress << ":"
				<< sourcePort << std::endl;

			sock.SendTo(echoBuffer, recvMsgSize, sourceAddress, sourcePort);
		}
	}
	catch (SocketException &e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}
	// NOT REACHED

	//return 0;
}

/*
*   C++ sockets on Unix and Windows
*   Copyright (C) 2002
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

int client_main(int argc, char *argv[])
{
	if ((argc < 3) || (argc > 4))
	{   // Test for correct number of arguments
		std::cerr << "Usage: " << argv[0]
			<< " <Server> <Echo String> [<Server Port>]\n";
		exit(1);
	}

	std::string servAddress = argv[1];             // First arg: server address
	char* echoString = argv[2];               // Second arg: string to echo
	int echoStringLen = strlen(echoString);   // Length of string to echo
	if (echoStringLen > ECHOMAX)
	{    // Check input length
		std::cerr << "Echo string too long" << std::endl;
		exit(1);
	}
	unsigned short echoServPort = Socket::ResolveService(
		(argc == 4) ? argv[3] : "echo", "udp");

	try
	{
		UDPSocket sock;

		// Send the string to the server
		sock.SendTo(echoString, echoStringLen, servAddress, echoServPort);

		// Receive a response
		char echoBuffer[ECHOMAX + 1];       // Buffer for echoed string + \0
		int respStringLen;                  // Length of received response
		if ((respStringLen = sock.Receive(echoBuffer, ECHOMAX)) != echoStringLen)
		{
			std::cerr << "Unable to receive" << std::endl;
			exit(1);
		}

		echoBuffer[respStringLen] = '\0';             // Terminate the string!
		std::cout << "Received: " << echoBuffer << std::endl;   // Print the echoed arg

													  // Destructor closes the socket

	}
	catch (SocketException &e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}

	return 0;
}


#include "SocketException.h"

int main(int argc, char *argv[])
{
	(argc);
	(argv);
	throw SocketException("Message!");
	/*
	if (argc == 2)
	{
		server_main(argc, argv);
	}
	else
	{
		client_main(argc, argv);
	}*/
}
	
/*	Parsing Code:

	std::vector<std::string> lines = ConfigReader::ReadFile("../Content/config.txt");
	
	assert(lines.size() % 3 == 0);
	for (int num = 0; num < (int)lines.size(); num += 3)
	{
		printf("WSACODE(%s\t,\"%s\",\"%s\")\n", lines[num].c_str(), lines[num + 1].c_str(), lines[num + 2].c_str());
	}
*/