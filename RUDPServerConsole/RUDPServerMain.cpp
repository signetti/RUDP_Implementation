#include "stdafx.h"
#include "SharedTestData.h"
#include "StopWatch.h"
#include "WSAManager.h"
#include <sstream>
#include <iostream>
#include <fstream>


#include <winsock2.h>
#include <ws2tcpip.h>

#include "RPacket.h"
#include "RUDPStream.h"
#include "RUDPServer.h"

#include <assert.h>
int BP(int condition)
{
	return condition;
}

int __cdecl main()
{
	// Re-seed
	srand(static_cast<unsigned int>(time(0)));

	// ============== Initialize Server Connection ==============
	bool success;

	// Create a server that is listening to the defined-port
	RUDPServer server(DEFAULT_SERVER_PORT_NUMBER, 1000);

	// Open the server for connection (create socket, bind, then listen)
	printf("Awaiting Client. . . \n");
	success = server.Open();
	if (!success) return BP(1);

	// Listen for and accept a client connection
	printf("Retrieving Client. . . \n");
	RUDPStream& client = *server.Accept();

	if (client.IsOpen() == false)
	{
		printf("Failed to create client connection.");
		return BP(0);
	}

	// Notify that connection is reached
	printf("Client-Server Connection Established.\n");


	// Create Protocol ID
	static const uint32_t BUF_SIZE = 12000;
	char recvbuf[BUF_SIZE];

	// Listen for incoming packets
	int bytesReceived;
	//int bytesSent;

	//struct sockaddr client_addr;
	RPacket data;
	printf("waiting on port %s\n", DEFAULT_SERVER_PORT);
	for (;;)
	{
		// Receiving Packets
		{
			bytesReceived = client.Receive(recvbuf, BUF_SIZE);
			if (bytesReceived <= 0)
			{
				printf("wut\n");
				continue;
			}

			std::string message(recvbuf, bytesReceived);
			printf("%s\n\n", message.c_str());

			printf("waiting on port %s\n", DEFAULT_SERVER_PORT);
		}
		/*
		// Receiving Packets
		{
			bytesReceived = client.Receive(recvbuf, DEFAULT_BUFLEN);
			if (bytesReceived <= 0)
			{
				continue;
			}

			// Convert bytes to RUDP packet information
			bool isSuccess = data.Deserialize(reinterpret_cast<uint8_t *>(recvbuf));

			if (!isSuccess)
			{
				printf("message: Not an RUDP packet\n");
			}
			else
			{
				printf("message [%d bytes]:\t(seq:%d ack:%d ack_bit:%d)\t%s\n"
					, bytesReceived, data.Sequence(), data.Ack(), data.AckBitfield(), data.Message().c_str());
			}

			printf("waiting on port %s\n", DEFAULT_SERVER_PORT);
		}*/
	}
}
