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

int __cdecl main()
{
	// Re-seed
	srand(static_cast<unsigned int>(time(0)));

	// ===================== Begin Connection to Server =====================
	RUDPStream server = RUDPClient::ConnectToServer(DEFAULT_IP, DEFAULT_SERVER_PORT, DEFAULT_CLIENT_PORT, 1500);

	if (server.IsOpen() == false)
	{
		printf("Failed to create server connection.");
		return BP(0);
	}

	printf("Created RUDP Server: socket<%d> address<%d.%d.%d.%d.%d.%d.%d.%d.%d.%d>\n", static_cast<int>(server.mSocket)
		, (uint8_t)(server.mToAddress.sa_data[0]), (uint8_t)(server.mToAddress.sa_data[1]), (uint8_t)(server.mToAddress.sa_data[2]), (uint8_t)(server.mToAddress.sa_data[3]), (uint8_t)(server.mToAddress.sa_data[4])
		, (uint8_t)(server.mToAddress.sa_data[5]), (uint8_t)(server.mToAddress.sa_data[6]), (uint8_t)(server.mToAddress.sa_data[7]), (uint8_t)(server.mToAddress.sa_data[8]), (uint8_t)(server.mToAddress.sa_data[9]));

	// Notify that connection is reached
	printf("Client-Server Connection Established.\n");

	for (;;)
	{
		/*
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
		*/

		// Send To Server
		int bytesSent;

		bytesSent = server.Send(TEST_MESSAGE);
		if (bytesSent < 0)
		{
			printf("Failed to send datagram");
			//return BP(1);
		}
		else
		{
			printf("%s\n", TEST_MESSAGE.c_str());
		}


		Sleep((rand() % 1000) + 2000);
	}
}

