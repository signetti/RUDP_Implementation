#include "stdafx.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int BP(int condition)
{
	return condition;
}

int __cdecl main()
{
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	bool success;

	// Create a server that is listening to the defined-port
	TCPServer server(DEFAULT_PORT);

	success = server.CreateSocket();
	if (!success) return BP(1);

	// Bind Listening Socket to port
	success = server.Bind();
	if (!success) return BP(1);

	// Listen in on port
	success = server.Listen();
	if (!success) return BP(1);

	// Listen for and accept a client socket
	TCPStream_ client = server.Accept();

	// Receive until the peer shuts down the connection
	int bytesReceived;
	int bytesSent;
	do
	{
		bytesReceived = client.Read(recvbuf, recvbuflen);
		if (bytesReceived > 0)
		{
			printf("Bytes received: %d\n %s\n", bytesReceived, recvbuf);

			// Echo the buffer back to the sender
			bytesSent = client.Write(recvbuf);
			printf("Bytes sent: %d\n", bytesSent);
		}
		else if (bytesReceived == 0)
			printf("Connection closed\n");
		else
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			client.Close();
			return BP(1);
		}
	} while (bytesReceived > 0);

	// Shutdown the connection since we're done
	success = client.Shutdown(SD_SEND);
	if (!success) return BP(0);

	// Close and Clean Up WinSockAPI
	client.Close();

	return BP(0);
}