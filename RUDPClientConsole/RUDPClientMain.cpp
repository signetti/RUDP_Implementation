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
#include "RUDPClient.h"

#include <assert.h>
int BP(int condition)
{
	return condition;
}

int GetError(SOCKET sock)
{
	int error = 0;
	socklen_t len = sizeof(error);
	int retval = getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len);

	if (retval != 0)
	{
		/* there was a problem getting the error code */
		printf("Error getting socket, error code: %d\n", WSAGetLastError());
	}
	return retval;
}

int __cdecl main()
{
	// Re-seed
	srand(static_cast<unsigned int>(time(0)));
	/*
	WSAManager::StartUp();
	// Create Socket
	SOCKET sock;
	sock = socket(AF_UNSPEC, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		printf("Could not create socket.\n");
		return BP(1);
	}

	// Get Address
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	// Resolve the server address and port
	struct addrinfo* mInfo;
	int result;

	result = getaddrinfo(DEFAULT_IP, DEFAULT_PORT, &hints, &mInfo);
	if (result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", result);
		mInfo = NULL;
		return BP(1);
	}

	// Create RUDPStream
	RUDPStream server(sock, *(mInfo->ai_addr));
	*/

	// ===================== Begin Connection to Server =====================
	RUDPStream server = RUDPClient::ConnectToServer(DEFAULT_IP, DEFAULT_PORT, 1500);

	if (server.IsOpen() == false)
	{
		printf("Failed to create server connection.");
		return BP(0);
	}
	GetError(server.mSocket);

	printf("Created RUDP Server: socket<%d> address<%d.%d.%d.%d.%d.%d.%d.%d.%d.%d>\n", server.mSocket
		, (uint8_t)(server.mToAddress.sa_data[0]), (uint8_t)(server.mToAddress.sa_data[1]), (uint8_t)(server.mToAddress.sa_data[2]), (uint8_t)(server.mToAddress.sa_data[3]), (uint8_t)(server.mToAddress.sa_data[4])
		, (uint8_t)(server.mToAddress.sa_data[5]), (uint8_t)(server.mToAddress.sa_data[6]), (uint8_t)(server.mToAddress.sa_data[7]), (uint8_t)(server.mToAddress.sa_data[8]), (uint8_t)(server.mToAddress.sa_data[9]));

	// Notify that connection is reached
	printf("Client-Server Connection Established.\n");

	for (;;)
	{
		// Create RUDP Packet
		RPacket packet(rand() % 100, rand() % 100, rand() % 100, TEST_MESSAGE);
		if (rand() % 5 == 0)
		{
			packet.BecomeBadPacket();
		}
		std::vector<uint8_t> serial = packet.Serialize();

		// Send To Server
		int bytesSent;

		bytesSent = server.Send(reinterpret_cast<const char *>(serial.data()), static_cast<int>(serial.size()));
		if (bytesSent < 0)
		{
			printf("Failed to send datagram");
			//return BP(1);
		}
		else
		{
			printf("message sent [%d bytes]:\t(id:%X seq:%d ack:%d ack_bit:%d)\t%s\n"
				, bytesSent, packet.Id(), packet.Sequence(), packet.Ack(), packet.AckBitfield(), packet.Message().c_str());
		}

		Sleep((rand() % 1000) + 2000);
	}
}

