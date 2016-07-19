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

int __cdecl main()
{
	std::vector<std::string> configLines = ConfigReader::ReadFile("../Content/config.txt");
	if (configLines.empty())
	{
		return BP(0);
	}
	const char * serverAddress = configLines[0].c_str();

	// Re-seed
	srand(static_cast<unsigned int>(time(0)));

	// ===================== Begin Connection to Server =====================
	
	try
	{
		///RUDPStream server = RUDPClient::ConnectToServer(server_ip, DEFAULT_SERVER_PORT_NUMBER, DEFAULT_SERVER_PORT_NUMBER + 1000, 1500);
		RUDPSocket server(DEFAULT_SERVER_PORT_NUMBER + 1000U, 1500);
		
		server.Connect(serverAddress, DEFAULT_SERVER_PORT_NUMBER);

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
			isSuccess = server.Send(TEST_MESSAGE.c_str(), static_cast<uint32_t>(TEST_MESSAGE.length()));
			if (isSuccess)
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

