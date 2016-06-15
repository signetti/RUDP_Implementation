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
	/*
	// Bind to Socket
	result = bind(sock, mInfo->ai_addr, (int)mInfo->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("ServerSocket: Failed to connect\n");
		return BP(1);
	}*/


	// Create Protocol ID
	//char sendbuf[DEFAULT_BUFLEN];
	//*((unsigned short *)sendbuf) = RELIABLE_UDP_ID;

	// Copy message
	//memcpy(&sendbuf[2],TEST_MESSAGE.c_str(), TEST_MESSAGE.length());

	// Create RUDP Packet
	srand(static_cast<unsigned int>(time(0)));
	RPacket packet(rand() % 100, rand() % 100, rand() % 100, TEST_MESSAGE);
	if (rand() % 2 == 0)
	{
		packet.BecomeBadPacket();
	}
	std::vector<uint8_t> serial = packet.Serialize();


	// Send To Server
	int server_length = sizeof(struct sockaddr_in);
	//int bytesReceived;
	int bytesSent;

	bytesSent = sendto(sock, reinterpret_cast<const char *>(serial.data()), static_cast<int>(serial.size()), 0, mInfo->ai_addr, server_length);
	if (bytesSent < 0)
	{
		printf("Failed to send datagram");
		return BP(1);
	}
	else
	{
		printf("message sent [%d bytes]:\t(id:%X seq:%d ack:%d ack_bit:%d)\t%s\n"
			, bytesSent, packet.Id(), packet.Sequence(), packet.Ack(), packet.AckBitfield(), packet.Message().c_str());
	}

	/*
	// Receive from
	bytesReceived = recvfrom(sock, recvbuf, STRLEN, 0, mInfo->ai_addr, &server_length);
	if (bytesReceived > 0)
	{
		printf("message [%d bytes]:\t %s\n", bytesReceived, recvbuf);
	}
	else 
	{
		printf("Failed to receive datagram");
		return BP(1);
	}*/

    return BP(0);
}

