#include "stdafx.h"
#include "SharedTestData.h"
#include "StopWatch.h"
#include <sstream>
#include <iostream>
#include <fstream>

#include "Logger.h"
#include "TCPSocket.h"
#include "UDPSocket.h"
#include "RUDPSocket.h"

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

	for (;;)
	{
		// ============== Initialize Server Connection ==============
		// Create a server that is listening to the defined-port
		std::shared_ptr<IServerSocket> server;

		switch (config.protocol)
		{
		case EProtocol::TCP:
			server = std::shared_ptr<IServerSocket>(new TCPServerSocket(DEFAULT_SERVER_PORT_NUMBER));
			break;
		case EProtocol::UDP:
			server = std::shared_ptr<IServerSocket>(new UDPServerSocket(DEFAULT_SERVER_PORT_NUMBER, config.maxTimeoutMS));
			break;
		case EProtocol::RUDP:
			server = std::shared_ptr<IServerSocket>(new RUDPServerSocket(DEFAULT_SERVER_PORT_NUMBER, config.maxTimeoutMS));
			break;
		}

		// Open the server for connection (create socket, bind, then listen)

		// Listen for and accept a client connection
		Logger::PrintF(__FILE__, "Awaiting Client. . . ");
		Socket * client = server->Accept();

		// Notify that connection is reached
		Logger::PrintF(__FILE__, "Client-Server Connection Established.\n");


		// ============= Assert Proper Test Conditions =============
		char recvbuf[DEFAULT_BUFLEN];
		int recvbuflen = DEFAULT_BUFLEN;

		int bytesReceived;

		bool isDataValid;

		for (;;)
		{
			// Send this application's Test Message to Client
			client->Send(config.message.c_str(), static_cast<uint32_t>(config.message.length() + 1U));

			// Get client's Test Message
			bytesReceived = client->Receive(recvbuf, recvbuflen);

			// Assert that the client is still connected
			if (bytesReceived <= 0)
			{
				//Logger::PrintF(__FILE__, "Client disconnected. Ending Test.\n");
				//return BP(1);
			}
			else
			{
				isDataValid = (bytesReceived >= 8) && (config.message == std::string(recvbuf));

				if (isDataValid) break;
				else
				{	// Client sent bad comparison
					Logger::PrintErrorF(__FILE__, "ERROR: Different test messages were detected from the client. This could cause problems during testing.\n");
					Logger::PrintF(__FILE__, "Press \'y\' to continue, or press any other key to try again.\n");

					if (getchar() != 'y')
					{
						break;
					}
				}

			}
		}

		// Send again to let client know to continue
		client->Send("yay!", 5);


		// =============== Begin Testing Connection ===============

		TimeRecorder<std::chrono::microseconds> record;
		int testNumber;

		std::ofstream logFile;

		// Initialize Log File Data
		logFile.open("test_results.csv");
		logFile << "Test Number, Bytes Received, Receive Time (탎), Send Time (탎), Error (1=error)";

		bool isTransmissionSuccessful;

		// Receive until the peer shuts down the connection
		for (testNumber = 1; testNumber <= NUM_OF_TEST_RUNS; ++testNumber)
		{
			// Time the receiving of the message from client
			{
				StopWatch<std::chrono::microseconds> time(record);
				do
				{
					bytesReceived = client->Receive(recvbuf, recvbuflen);
				} while (config.isSendOnSuccess && bytesReceived == 0);
			}

			// Time the echo of the message to the client
			{
				StopWatch<std::chrono::microseconds> time(record);
				do
				{
					isTransmissionSuccessful = client->Send(config.message.c_str(), static_cast<uint32_t>(config.message.length() + 1U));
				} while (config.isSendOnSuccess && !isTransmissionSuccessful);
			}


			// Check Validity of Data received
			isDataValid = (bytesReceived != 0) && (config.message == std::string(recvbuf));

			if (!isDataValid)
			{
				isDataValid = isDataValid;
			}

			// Test Complete, Store / Print Results
			std::stringstream results;
			std::vector<std::chrono::microseconds> times = record.GetTimeRecordings(true);
			assert(record.Size() == 0);

			// Print to Log File
			logFile << '\n' << testNumber << ',' << bytesReceived << ','
				<< times[0].count() << ',' << times[1].count() << ','
				<< ((isDataValid) ? 0 : 1);

			// Print to Console
			results << "Reply from client: bytes=" << bytesReceived
				<< " recv_time=" << times[0].count()
				<< "us send_time=" << times[1].count()
				<< "us error=" << ((!isDataValid) ? "true" : "false") << '\n';
			Logger::PrintF(__FILE__, results.str().c_str());
		}

		// Close File with Calculated Results
		logFile << "\n\n,,Avg Receive Time (탎), Avg Send Time (탎),Percent of Error\n,"
			<< ",=SUM(C2:C" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS
			<< ",=SUM(D2:D" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS
			<< ",=SUM(E2:E" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS;
		logFile.close();


		// ================= End Test On Server-Side ===============
		Logger::PrintF(__FILE__, "\nTest Complete.\n");
		client->Close();
	}

	//return BP(0);
}