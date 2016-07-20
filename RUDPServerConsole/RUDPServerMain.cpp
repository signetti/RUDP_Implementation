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

#define config config	// Simply used for identifying config's effect in code (using Visual Studio text color)

int __cdecl main()
{
	// ========================== Parse Config File =========================
	config_t config;
	if (!ConfigReader::ParseConfig(config, CONFIG_FILE_PATH))
	{
		return BP(0);
	}

	// Re-seed
	srand(static_cast<unsigned int>(time(0)));

	// ============== Initialize Server Connection ==============

	// Create a server that is listening to the defined-port
	RUDPServerSocket server(DEFAULT_SERVER_PORT_NUMBER, 1500);

	// Listen for and accept a client connection
	Logger::PrintF(__FILE__, "Awaiting Client. . . \n");
	RUDPSocket& client = *server.Accept();

	if (client.IsOpen() == false)
	{
		Logger::PrintF(__FILE__, "Failed to create client connection.");
		return BP(0);
	}

	// Notify that connection is reached
	Logger::PrintF(__FILE__, "Client-Server Connection Established.\n");


	// Create Protocol ID
	static const uint32_t BUF_SIZE = 12000;
	char recvbuf[BUF_SIZE];

	// Listen for incoming packets
	int bytesReceived;

	//struct sockaddr client_addr;
	RPacket data;
	Logger::PrintF(__FILE__, "waiting on port %d\n", DEFAULT_SERVER_PORT_NUMBER);
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
				for (auto letter : config.message)
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
					assert(message != config.message);
					Logger::PrintF(__FILE__, ".... ERROR: Received Message does not matches Expected Message\ncharacter %2d: \t...%s...\n\n", index
						, message.substr(((index - 50 >= 0) ? index - 50 : 0),((index + 50 < msg_size) ? index + 50 : msg_size - 1)).c_str());
				}
				else
				{
					assert(message == config.message);
					Logger::PrintF(__FILE__, "==== Received Message matches Expected Message ====\n\n");
				}
			}
		}
	}
}
