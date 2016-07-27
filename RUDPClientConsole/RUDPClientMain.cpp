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
///#include "RUDPClient.h"
#include "RUDPSocket.h"

#include "ConfigReader.h"
#include "Logger.h"
#include "SocketException.h"

#include <assert.h>
int BP(int condition)
{
	getchar();
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

	// ===================== Begin Connection to Server =====================
	
	try
	{
		///RUDPStream server = RUDPClient::ConnectToServer(server_ip, DEFAULT_SERVER_PORT_NUMBER, DEFAULT_SERVER_PORT_NUMBER + 1000, 1500);
		RUDPSocket server(DEFAULT_SERVER_PORT_NUMBER + 1000U, 1500);
		
		server.Connect(config.serverAddress, DEFAULT_SERVER_PORT_NUMBER);

		if (server.IsOpen() == false)
		{
			Logger::PrintF(__FILE__, "Failed to create server connection.");
			return BP(0);
		}

		// Notify that connection is reached
		Logger::PrintF(__FILE__, "Client-Server Connection Established.\n");

		do
		{
			// Send To Server
			bool isSuccess;

			///bytesSent = server.Send(TEST_MESSAGE);
			isSuccess = server.Send(config.message.c_str(), static_cast<uint32_t>(config.message.length()));
			if (!isSuccess)
			{
				Logger::PrintF(__FILE__, "Failed to send datagram\n");
				//return BP(1);
			}


			Sleep((rand() % 1000) + 2000);
		} while (getchar() != 'q');
	}
	catch (SocketException ex)
	{
		Logger::PrintError(__FILE__, ex.what());
		return BP(0);
	}
}

