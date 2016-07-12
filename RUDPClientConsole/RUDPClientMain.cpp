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

#include "ConfigReader.h"

#include <assert.h>
int BP(int condition)
{
	getchar();
	return condition;
}

int __cdecl main()
{
	std::vector<std::string> configLines = ConfigReader::ReadFile("../Content/config.txt");
	if (configLines.empty())
	{
		return BP(0);
	}
	const char * server_ip = configLines[0].c_str();

	// Re-seed
	srand(static_cast<unsigned int>(time(0)));

	// ===================== Begin Connection to Server =====================
	RUDPStream server = RUDPClient::ConnectToServer(server_ip, DEFAULT_SERVER_PORT_NUMBER, DEFAULT_SERVER_PORT_NUMBER + 1000, 1500);

	if (server.IsOpen() == false)
	{
		printf("Failed to create server connection.");
		return BP(0);
	}

	// Notify that connection is reached
	printf("Client-Server Connection Established.\n");

	do
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
			printf("Failed to send datagram\n");
			//return BP(1);
		}
		else
		{
			printf("%s\n\n", TEST_MESSAGE.c_str());
		}


		Sleep((rand() % 1000) + 2000);
	} while (getchar() != 'q');
}

