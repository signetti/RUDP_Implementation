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
///#include "RUDPStream.h"
///#include "RUDPServer.h"
#include "RUDPSocket.h"

#include "Logger.h"

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

	// Create a server that is listening to the defined-port
	///RUDPServer server(DEFAULT_SERVER_PORT_NUMBER, 1500);
	RUDPServerSocket server(DEFAULT_SERVER_PORT_NUMBER, 1500);

	// Listen for and accept a client connection
	Logger::PrintF("Awaiting Client. . . \n");
	///RUDPStream& client = *server.Accept();
	RUDPSocket& client = *server.Accept();

	if (client.IsOpen() == false)
	{
		Logger::PrintF("Failed to create client connection.");
		return BP(0);
	}

	// Notify that connection is reached
	Logger::PrintF("Client-Server Connection Established.\n");


	// Create Protocol ID
	static const uint32_t BUF_SIZE = 12000;
	char recvbuf[BUF_SIZE];

	// Listen for incoming packets
	int bytesReceived;
	//int bytesSent;

	//struct sockaddr client_addr;
	RPacket data;
	Logger::PrintF("waiting on port %s\n", DEFAULT_SERVER_PORT);
	for (;;)
	{
		// Receiving Packets
		{
			bytesReceived = client.Receive(recvbuf, BUF_SIZE);
			if (bytesReceived > 0)
			{
				// Minus 1 due to end-of-line byte...
				std::string message(recvbuf, bytesReceived);

				int index = 0;
				int msg_size = (int)message.length();
				for (auto letter : TEST_MESSAGE)
				{
					if (index >= msg_size)
					{
						break;
					}
					if (letter != message[index])
					{
						break;
					}
					index++;
				}

				if (index < msg_size)
				{
					assert(message != TEST_MESSAGE);
					Logger::PrintF(".... ERROR: Received Message does not matches Expected Message\ncharacter %2d: \t...%s...\n\n", index
						, message.substr(((index - 50 >= 0) ? index - 50 : 0),((index + 50 < msg_size) ? index + 50 : msg_size - 1)).c_str());
				}
				else
				{
					assert(message == TEST_MESSAGE);
					Logger::PrintF("==== Received Message matches Expected Message ====\n\n");
				}
			}
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
				Logger::PrintF("message: Not an RUDP packet\n");
			}
			else
			{
				Logger::PrintF("message [%d bytes]:\t(seq:%d ack:%d ack_bit:%d)\t%s\n"
					, bytesReceived, data.Sequence(), data.Ack(), data.AckBitfield(), data.Message().c_str());
			}

			Logger::PrintF("waiting on port %s\n", DEFAULT_SERVER_PORT);
		}*/
	}
}
