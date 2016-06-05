#include "stdafx.h"
#include "SharedTestData.h"
//#define DEFAULT_BUFLEN	512
//#define DEFAULT_PORT	"27015"
//#define DEFAULT_IP		"10.8.3.35"

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
	bytesReceived = server.Read(recvbuf, recvbuflen);
	
	do
	{
		// Check Validity of Test Message received
		isDataValid = static_cast<int>(TEST_MESSAGE.length()+1) == bytesReceived;
		isDataValid = isDataValid && (TEST_MESSAGE.compare(recvbuf) == 0);

		if (isDataValid)
		{
			// Server's Test Message is Valid!
			// Send this application's Test Message to Server
			server.Write(TEST_MESSAGE.c_str());

			// Get response from Server
			bytesReceived = server.Read(recvbuf, recvbuflen);

			// If bytes received is a large number, then it's a test message.
			// Else server responded successful matching Test Message
			isDataValid = (bytesReceived < 8);
		}
		else
		{
			// Server's Test Message is invalid
			// Send bad data to Server
			server.Write("boo!");

			// Get Test message again from the Server
			bytesReceived = server.Read(recvbuf, recvbuflen);
		}

		// Assert that the server is still connected
		if (bytesReceived < 0)
		{
			printf("Server disconnected. Ending Test.");
			return BP(1);
		}
	} while (!isDataValid);


	// =============== Begin Testing Connection ===============
	int testNumber;

	// Receive until the peer shuts down the connection
	for (testNumber = 1; bytesReceived > 0 && testNumber <= NUM_OF_TEST_RUNS; ++testNumber)
	{
		// Send Test Message to Server
		server.Write(TEST_MESSAGE.c_str());

		// Receive Test Message from Server
		bytesReceived = server.Read(recvbuf, recvbuflen);

		if (bytesReceived <= 0)
		{
			printf("Server disconnected mid-test? Ending Test.");
			return BP(1);
		}
	}

	// ================= End Test On Client-Side ===============
	printf("\nTest Complete.");

	// Close and Clean Up WinSockAPI
	server.Close();

	return BP(0);
}
