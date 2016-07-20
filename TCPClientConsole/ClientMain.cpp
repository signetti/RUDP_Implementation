#include "stdafx.h"
#include "SharedTestData.h"
#include "StopWatch.h"
#include <sstream>
#include <iostream>
#include <fstream>

#include "ConfigReader.h"

#include "Logger.h"

#include "TCPSocket.h"
#include "RUDPSocket.h"

#include <assert.h>
#include <memory>


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

	// ===================== Begin Connection to Server =====================
	std::shared_ptr<Socket> server;

	switch (config.protocol)
	{
	case EProtocol::TCP:
		server = std::shared_ptr<Socket>(new TCPSocket());
		break;
	case EProtocol::UDP:
		server = std::shared_ptr<Socket>(new UDPSocket(DEFAULT_CLIENT_PORT_NUMBER, config.maxTimeoutMS));
		break;
	case EProtocol::RUDP:
		server = std::shared_ptr<Socket>(new RUDPSocket(DEFAULT_CLIENT_PORT_NUMBER, config.maxTimeoutMS));
		break;
	}

	bool isSuccess = server->Connect(config.serverAddress, DEFAULT_SERVER_PORT_NUMBER);
	if (!isSuccess)
	{
		Logger::PrintErrorF(__FILE__, "Error in creating Client-Server Connection.\n");
		return BP(0);
	}

	// Notify that connection is reached
	Logger::PrintF(__FILE__, "Client-Server Connection Established.\n");


	// ============= Assert Proper Test Conditions =============
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	uint32_t bytesReceived;
	bool isDataValid;
	
	// Get Server's Test message (at least once)
	bytesReceived = server->Receive(recvbuf, recvbuflen);
	
	do
	{
		// Check Validity of Test Message received
		isDataValid = (config.message == std::string(recvbuf));

		if (isDataValid)
		{	// Server's Test Message is Valid!

			// Send this application's Test Message to Server
			server->Send(config.message.c_str(), static_cast<uint32_t>(config.message.length() + 1U));

			// Get response from Server
			bytesReceived = server->Receive(recvbuf, recvbuflen);

			// If bytes received is a large number, then it's a test message.
			// Else server responded successful matching Test Message
			isDataValid = (bytesReceived < 8);
		}
		else
		{	// Server's Test Message is invalid

			// Send bad data to Server
			server->Send("boo!", 5);

			// Get Test message again from the Server
			bytesReceived = server->Receive(recvbuf, recvbuflen);
		}

		// Assert that the server is still connected
		if (bytesReceived < 0)
		{
			Logger::PrintF(__FILE__, "Server disconnected. Ending Test.\n");
			return BP(1);
		}

	} while (!isDataValid);


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
		// Time the sending of the message to the server
		{
			StopWatch<std::chrono::microseconds> time(record);
			do
			{
				isTransmissionSuccessful = server->Send(config.message.c_str(), static_cast<uint32_t>(config.message.length() + 1));
			} while (config.isSendOnSuccess && !isTransmissionSuccessful);
		}

		// Time the echo of the message from the server
		{
			StopWatch<std::chrono::microseconds> time(record);
			do
			{
				bytesReceived = server->Receive(recvbuf, recvbuflen);
			} while (config.isSendOnSuccess && bytesReceived == 0);
		}

		// Check Validity of Data received
		isDataValid = bytesReceived != 0 && (config.message == std::string(recvbuf));

		// Test Complete, Store / Print Results
		std::stringstream results;
		std::vector<std::chrono::microseconds> times = record.GetTimeRecordings(true);
		assert(record.Size() == 0);

		// Print to Log File
		logFile << '\n' << testNumber << ',' << bytesReceived << ',' << times[0].count() << ',' << times[1].count() << ',' << ((isDataValid) ? 0 : 1);

		results << "Reply from server: bytes=" << bytesReceived
				<< " recv_time="					 << times[0].count()
				<< "us send_time="					 << times[1].count()
				<< "us error="						 << ((!isDataValid) ? "true" : "false") << '\n';

		Logger::PrintF(__FILE__, results.str().c_str());
	}

	// Close File with Calculated Results
	logFile << "\n\n,,Avg Receive Time (탎), Avg Send Time (탎),Percent of Error\n,"
		<< ",=SUM(C2:C" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS
		<< ",=SUM(D2:D" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS
		<< ",=SUM(E2:E" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS;
	logFile.close();

	// ================= End Test On Client-Side ===============
	Logger::PrintF(__FILE__, "\nTest Complete.\n");
	server->Close();

	return BP(0);
}
