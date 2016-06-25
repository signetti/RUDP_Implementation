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
	// ============== Initialize Server Connection ==============
	bool success;

	// Create a server that is listening to the defined-port
	TCPServer server(DEFAULT_SERVER_PORT);

	// Open the server for connection (create socket, bind, then listen)
	success = server.Open();
	if (!success) return BP(1);

	// Listen for and accept a client connection
	printf("Awaiting Client. . . ");
	TCPStream client = server.Accept();

	// Notify that connection is reached
	printf("Client-Server Connection Established.");


	// ============= Assert Proper Test Conditions =============
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	int bytesReceived;

	bool isDataValid;

	do
	{
		// Send this application's Test Message to Client
		client.Send(TEST_MESSAGE.c_str());

		// Get client's Test Message
		bytesReceived = client.Receive(recvbuf, recvbuflen);

		// Assert that the client is still connected
		if (bytesReceived <= 0)
		{
			printf("\nClient disconnected. Ending Test.");
			return BP(1);
		}
		else if (bytesReceived < 8)
		{
			// Client sent bad comparison
			printf("\nERROR: Different test messages were detected from the client. This could cause problems during testing.\
					\nPress \'y\' to continue, or press any other key to try again.");
		}
		else
		{
			// Check Validity of Data received
			isDataValid = ValidateMessageReceived(recvbuf, bytesReceived);

			if (isDataValid) break;
			else
				printf("\nERROR: Different test messages were detected. This could cause problems during testing.\
						\nPress \'y\' to continue, or press any other key to try again.");
		}
	} while(getchar() != 'y');

	// Send again to let client know to continue
	client.Send("yay!");


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
		// Time the receiving of the message from client
		{
			StopWatch<std::chrono::microseconds> time(record);
			bytesReceived = client.Receive(recvbuf, recvbuflen);
		}

		// Time the echo of the message to the client
		if (bytesReceived > 0)
		{
			StopWatch<std::chrono::microseconds> time(record);
			client.Send(recvbuf);
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
			logFile << '\n' << testNumber << ',' << bytesReceived << ','
					<< times[0].count() << ',' << times[1].count() << ','
					<< ((isDataValid) ? 0 : 1);
			
			// Print to Console
			results << "\nReply from client: bytes=" << bytesReceived
					<< " recv_time="				 << times[0].count()
					<< "us send_time="				 << times[1].count()
					<< "us error="					 << ((!isDataValid) ? "true" : "false");
			printf(results.str().c_str());
		}
		else if (bytesReceived == 0)
		{
			printf("\nConnection closed");
		}
		else
		{
			printf("\nrecv failed with error: %d", WSAGetLastError());
			client.Close();
			return BP(1);
		}
	}

	// Close File with Calculated Results
	logFile << "\n\n,,Avg Receive Time (탎), Avg Send Time (탎),Percent of Error\n,"
			<< ",=SUM(C2:C" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS
			<< ",=SUM(D2:D" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS
			<< ",=SUM(E2:E" << (NUM_OF_TEST_RUNS + 1) << ")/" << NUM_OF_TEST_RUNS;
	logFile.close();


	// ================= End Test On Server-Side ===============
	printf("\nTest Complete.");

	// Shutdown the connection since we're done
	success = client.Shutdown(SD_SEND);
	if (!success) return BP(0);

	// Close and Clean Up WinSockAPI
	client.Close();

	return BP(0);
}