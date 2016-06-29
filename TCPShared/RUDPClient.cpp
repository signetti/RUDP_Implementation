#include "stdafx.h"
#include "RUDPClient.h"
#include "RUDPStream.h"
#include "RPacket.h"

RUDPStream RUDPClient::Connect(const SOCKET & serverSocket, const sockaddr & serverAddress, uint32_t maxConnectionTimeOut)
{
	/** Perform Three-way Hand-shaking
	*	This works as follows
	*	 - Client sends message with SeqNum X
	*	 - Server receives message and returns message with SeqNum Y and AckNum X+1
	*	 - Client receives, checks AckNum for X+1, and returns message with AckNum Y+1
	*	 - Server receives message, checks AckNum for Y+1, and
	*	Returning the Num+1 tells the receiving end that the number was recognized.
	*/

	// Before beginning the process, we must make the recvfrom non-blocking
	// This is to check on a time-out period to deliver the message again.
	static DWORD NON_BLOCKING = TRUE;
	if (ioctlsocket(serverSocket, FIONBIO, &NON_BLOCKING) != 0)
	{
		printf("failed to set non-blocking\n");
		return RUDPStream(INVALID_SOCKET);
	}

	// Generate Initial Sequence Number
	uint32_t seqNum = rand();

	// Generate Request Packet
	std::vector<uint8_t> acknowledgePacket = RPacket::SerializeInstance(seqNum, 0, 0, 0, 0, std::vector<uint8_t>());

	// Begin Establishing Connection
	RPacket data;
	int32_t bytesSent;
	int32_t bytesReceived;
	char buffer[512];
	sockaddr fromAddress;
	bool isSuccess;
	static int ADDR_LEN = sizeof(struct sockaddr_in);
	for (;;)
	{
		// Send Request Packet
		bytesSent = sendto(serverSocket, reinterpret_cast<char *>(acknowledgePacket.data()), static_cast<int>(acknowledgePacket.size()), 0, &serverAddress, ADDR_LEN);
		if (bytesSent < 0)
		{
			printf("sendto failed with error: %d\n", WSAGetLastError());
			return RUDPStream(INVALID_SOCKET);
		}

		// Begin Timing (Record Time it was Sent)
		std::chrono::high_resolution_clock::time_point timeSentToServer = std::chrono::high_resolution_clock::now();
		printf("Establishing Connection: Sending  seq <%d> ack<%d>\n", seqNum, 0);

		for (;;)
		{
			// Get Incoming Packets
			bytesReceived = recvfrom(serverSocket, buffer, 512, 0, &fromAddress, &ADDR_LEN);
			//bytesReceived = Select(buffer,512, maxConnectionTimeOut);

			// Check if Packet is Received
			if (bytesReceived > 0)
			{	// Packet has been received!

				// Check if packet's address is from server
				if (strcmp(&fromAddress.sa_data[2], &serverAddress.sa_data[2]) != 0)
				{	// Packet is not from the server, ignore...
					printf("RUDP Packet is not from Server: expected<%d.%d.%d.%d.%d.%d.%d.%d.%d.%d> actual<%d.%d.%d.%d.%d.%d.%d.%d.%d.%d>\n"
						, (uint8_t)(serverAddress.sa_data[0]), (uint8_t)(serverAddress.sa_data[1]), (uint8_t)(serverAddress.sa_data[2]), (uint8_t)(serverAddress.sa_data[3]), (uint8_t)(serverAddress.sa_data[4])
						, (uint8_t)(serverAddress.sa_data[5]), (uint8_t)(serverAddress.sa_data[6]), (uint8_t)(serverAddress.sa_data[7]), (uint8_t)(serverAddress.sa_data[8]), (uint8_t)(serverAddress.sa_data[9])
						, (uint8_t)(fromAddress.sa_data[0]), (uint8_t)(fromAddress.sa_data[1]), (uint8_t)(fromAddress.sa_data[2]), (uint8_t)(fromAddress.sa_data[3]), (uint8_t)(fromAddress.sa_data[4])
						, (uint8_t)(fromAddress.sa_data[5]), (uint8_t)(fromAddress.sa_data[6]), (uint8_t)(fromAddress.sa_data[7]), (uint8_t)(fromAddress.sa_data[8]), (uint8_t)(fromAddress.sa_data[9]));
					continue;
				}

				// Convert bytes to RUDP packet information
				isSuccess = data.Deserialize(reinterpret_cast<uint8_t *>(buffer));

				// Check if Packet is RUDP Packet
				if (!isSuccess)
				{	// Packet is not an RUDP Packet, ignore...
					printf("Packet is not an RUDP Packet\n");
					continue;
				}

				// Packet is an RUDP Packet from Server, accept Packet!
				break;
			}
			else if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				printf("recvfrom produced error: %ld\n", WSAGetLastError());
			}

			// Sleep for a very short period, so as not to take up CPU power
			Sleep(1);

			// Check if Request timed-out
			if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timeSentToServer).count() >= maxConnectionTimeOut)
			{
				// Establishing connection timed-out. Return unsuccessful.
				printf("Establishing Connection Timed Out.\n");
				return RUDPStream(INVALID_SOCKET);
			}
		}

		// Check if Acknowledge Number is your Sequence Number
		if (data.Ack() != seqNum + 1)
		{	// Bad Acknowledgement number, send again!
			printf("Bad Acknowledgement number: expected<%d> actual<%d>\n", seqNum + 1, data.Ack());
			continue;
		}

		// Acknowledgement Successful!
		printf("Acknowledged Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());
		break;
	}

	// Send Acknowledgement to Complete Three-Way Handshake
	seqNum += 1;
	uint32_t ackNum = data.Sequence() + 1;
	acknowledgePacket = RPacket::SerializeInstance(seqNum, ackNum, 0, 0, 0, std::vector<uint8_t>());

	bytesSent = sendto(serverSocket, reinterpret_cast<char *>(acknowledgePacket.data()), static_cast<int>(acknowledgePacket.size()), 0, &serverAddress, ADDR_LEN);
	if (bytesSent < 0)
	{
		printf("sendto failed with error: %d\n", WSAGetLastError());
		return RUDPStream(INVALID_SOCKET);
	}

	// At this point the connection is established
	printf("Acknowledging Connection: Sending  seq <%d> ack<%d>\n", seqNum, ackNum);

	return RUDPStream(serverSocket, fromAddress, seqNum, ackNum, maxConnectionTimeOut);
	//return RUDPStream(serverSocket, serverAddress, seqNum, ackNum, maxConnectionTimeOut);
}

RUDPStream RUDPClient::ConnectToServer(char * ip, char * port, char * clientPort, uint32_t maxConnectionTimeOut)
{
	struct addrinfo hints;
	struct addrinfo * info;

	SOCKET serverSocket = INVALID_SOCKET;

	// Start-Up WSA
	WSAManager::StartUp();

	// Get Address Information from the given IP and Port
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	int result;

	// Resolve the server address and port
	result = getaddrinfo(ip, port, &hints, &info);
	if (result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", result);
		return RUDPStream(INVALID_SOCKET);
	}

	// Create a SOCKET for connecting to server
	serverSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (serverSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		return RUDPStream(INVALID_SOCKET);
	}
	
	// Resolve this Client's address and port (to bind with)
	struct addrinfo * clientInfo;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;	// Get Address Information from this computer

	// Resolve the client's address and port
	result = getaddrinfo(NULL, clientPort, &hints, &clientInfo);
	if (result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", result);
		closesocket(serverSocket);
		return RUDPStream(INVALID_SOCKET);
	}

	// Bind Port to Socket
	result = bind(serverSocket, clientInfo->ai_addr, (int)clientInfo->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		return RUDPStream(INVALID_SOCKET);
	}

	// Check for Connection
	struct sockaddr& serverAddress = *(info->ai_addr);

	// Attempt to Connect, return results
	return Connect(serverSocket, serverAddress, maxConnectionTimeOut);
}