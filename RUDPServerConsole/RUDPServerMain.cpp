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
	/*
	WSAManager::StartUp();

	// Create Socket
	SOCKET sock;
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		printf("Could not create socket.\n");
		return BP(1);
	}

	struct addrinfo hints;
	struct addrinfo * info;

	// Get Address Information from the given IP and Port
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	int result;

	// Resolve the server address and port
	result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &info);
	if (result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", result);
		return BP(0);
	}

	// Bind to Socket
	result = bind(sock, info->ai_addr, (int)info->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("ServerSocket: Failed to connect\n");
		return BP(1);
	}
	freeaddrinfo(info);

	// Create RUDPStream
	RUDPStream client(sock);
	*/

	// ============== Initialize Server Connection ==============
	bool success;

	// Create a server that is listening to the defined-port
	RUDPServer server(DEFAULT_PORT, 1000);

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
	char recvbuf[DEFAULT_BUFLEN];

	// Listen for incoming packets
	int bytesReceived;
	//int bytesSent;

	//struct sockaddr client_addr;
	RPacket data;
	printf("waiting on port %s\n", DEFAULT_PORT);
	for (;;)
	{
		// Sending Packets
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

			printf("waiting on port %s\n", DEFAULT_PORT);
		}
		/*
		// Receiving Packets
		{
			GetError3(client.mSocket);
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

			printf("waiting on port %s\n", DEFAULT_PORT);
		}*/
	}
}
