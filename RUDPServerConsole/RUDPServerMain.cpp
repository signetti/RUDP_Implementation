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

#include <assert.h>
int BP(int condition)
{
	return condition;
}

int __cdecl main()
{
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

	// Create Protocol ID
	char recvbuf[DEFAULT_BUFLEN];

	// Listen for incoming packets
	int bytesReceived;
	//int bytesSent;

	struct sockaddr client_addr;
	int client_addr_len = sizeof(struct sockaddr_in);
	//char * message;

	for (;;)
	{
		printf("waiting on port %s\n", DEFAULT_PORT);
		bytesReceived = recvfrom(sock, recvbuf, DEFAULT_BUFLEN, 0, &client_addr, &client_addr_len);
		if (bytesReceived > 0)
		{
			// Convert bytes to RUDP packet information
			RPacket * data = RPacket::Deserialize(reinterpret_cast<uint8_t *>(recvbuf), DEFAULT_BUFLEN);
			assert(data != nullptr);

			if (data->IsBadPacket())
			{
				printf("message from protocol: 0x%X\n", data->Id());
			}
			else
			{
				printf("message [%d bytes]:\t(seq:%d ack:%d ack_bit:%d)\t%s\n"
					, bytesReceived, data->Sequence(), data->Ack(), data->AckBitfield(), data->Message().c_str());
			}

			/*
			// Send Acknowledgement
			bytesSent = sendto(sock, recvbuf, (int)strlen(recvbuf) + 1, 0, &client_addr, client_addr_len);
			if (bytesSent < 0)
			{
				printf("Failed to send acknowledgement");
				return BP(1);
			}*/
		}
		else
		{
			printf("received bytes is %d?", bytesReceived);
		}
	}
}
