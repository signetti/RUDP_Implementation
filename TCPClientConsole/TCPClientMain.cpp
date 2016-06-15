#include "stdafx.h"
#include "SharedTestData.h"
#include "StopWatch.h"
#include <sstream>
#include <iostream>
#include <fstream>

#include <assert.h>
int BP(int condition)
{
	return condition;
}

int __cdecl main()
{
	// ===================== Begin Connection to Server =====================
	TCPStream server = TCPClient::ConnectToServer(DEFAULT_IP, DEFAULT_PORT);

	// Notify that connection is reached
	printf("Client-Server Connection Established.");


	// ============= Assert Proper Test Conditions =============
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	int bytesReceived;
	bool isDataValid;
	
	// Get Server's Test message (at least once)
	bytesReceived = server.Receive(recvbuf, recvbuflen);
	
	do
	{
		// Check Validity of Test Message received
		isDataValid = ValidateMessageReceived(recvbuf, bytesReceived);

		if (isDataValid)
		{
			// Server's Test Message is Valid!
			// Send this application's Test Message to Server
			server.Send(TEST_MESSAGE.c_str());

			// Get response from Server
			bytesReceived = server.Receive(recvbuf, recvbuflen);

			// If bytes received is a large number, then it's a test message.
			// Else server responded successful matching Test Message
			isDataValid = (bytesReceived < 8);
		}
		else
		{
			// Server's Test Message is invalid
			// Send bad data to Server
			server.Send("boo!");

			// Get Test message again from the Server
			bytesReceived = server.Receive(recvbuf, recvbuflen);
		}

		// Assert that the server is still connected
		if (bytesReceived < 0)
		{
			printf("Server disconnected. Ending Test.");
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

	// Receive until the peer shuts down the connection
	for (testNumber = 1; bytesReceived > 0 && testNumber <= NUM_OF_TEST_RUNS; ++testNumber)
	{
		// Time the sending of the message to the server
		{
			StopWatch<std::chrono::microseconds> time(record);
			server.Send(TEST_MESSAGE.c_str());
		}

		// Time the echo of the message from the server
		if (bytesReceived > 0)
		{
			StopWatch<std::chrono::microseconds> time(record);
			bytesReceived = server.Receive(recvbuf, recvbuflen);
		}

		if (bytesReceived <= 0)
		{
			printf("Server disconnected mid-test? Ending Test.");
			return BP(1);
		}

		// Check Validity of Data received
		isDataValid = ValidateMessageReceived(recvbuf, bytesReceived);

		// Test Complete, Store / Print Results
		if (bytesReceived > 0)
		{
			std::stringstream results;
			std::vector<std::chrono::microseconds> times = record.GetTimeRecordings(true);
			assert(record.Size() == 0);

			// Print to Log File
			logFile << '\n' << testNumber << ',' << bytesReceived << ',' << times[0].count() << ',' << times[1].count() << ',' << ((isDataValid) ? 0 : 1);

			results << "\nReply from server: bytes=" << bytesReceived
				<< " recv_time="					 << times[0].count()
				<< "us send_time="					 << times[1].count()
				<< "us error="						 << ((!isDataValid) ? "true" : "false");
			printf(results.str().c_str());
		}
		else if (bytesReceived == 0)
			printf("\nConnection closed");
		else
		{
			printf("\nrecv failed with error: %d", WSAGetLastError());
			server.Close();
			return BP(1);
		}
	}

	// Close File with Calculated Results
	logFile << "\n\n,,Avg Receive Time (탎), Avg Send Time (탎),Percent of Error\n,"
		<< ",=SUM(C2:C" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS
		<< ",=SUM(D2:D" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS
		<< ",=SUM(E2:E" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS;
	logFile.close();

	// ================= End Test On Client-Side ===============
	printf("\nTest Complete.");

	// Close and Clean Up WinSockAPI
	server.Close();

	return BP(0);
}
