#include "stdafx.h"

#define DEFAULT_BUFLEN	512
#define DEFAULT_PORT	"27015"
#define DEFAULT_IP		"10.8.3.35"

int BP(int condition)
{
	return condition;
}

int __cdecl main()
{
	char *sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	bool success;

	TCPStream_ server = TCPClient::ConnectToServer(DEFAULT_IP, DEFAULT_PORT);

	// Send an initial buffer through the Socket
	int bytesSent = server.Write(sendbuf);
	printf("Bytes Sent: %ld\n", bytesSent);

	// Shutdown the connection since no more data will be sent
	success = server.Shutdown(SD_SEND);
	if (!success) return BP(1);

	// Receive until the peer closes the connection
	int bytesReceived;
	do
	{
		// Listen for incoming data
		bytesReceived = server.Read(recvbuf, recvbuflen);

		// Print according to the received result
		if (bytesReceived > 0)
			printf("Bytes received: %d\n", bytesReceived);
		else if (bytesReceived == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());
	} while (bytesReceived > 0);

	// Close and Clean Up WinSockAPI
	server.Close();

	return BP(0);
}
