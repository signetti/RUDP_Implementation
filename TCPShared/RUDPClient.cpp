#include "stdafx.h"
#include "RUDPClient.h"
#include "RUDPStream.h"
#include "RPacket.h"

#include "UDPSocket.h"
#include "SocketException.h"

#include "Logger.h"

RUDPStream RUDPClient::ConnectToServer(const char * ip, unsigned short port, unsigned short clientPort, uint32_t maxConnectionTimeOut)
{
	std::string serverAddress(ip);
	unsigned short serverPort(port);

	std::shared_ptr<UDPSocket> serverSocket = std::make_shared<UDPSocket>(clientPort);

	/** Perform Three-way Hand-shaking
	*	This works as follows
	*	 - Client sends message with SeqNum X
	*	 - Server receives message and returns message with SeqNum Y and AckNum X+1
	*	 - Client receives, checks AckNum for X+1, and returns message with AckNum Y+1
	*	 - Server receives message, checks AckNum for Y+1, and
	*	Returning the Num+1 tells the receiving end that the number was recognized.
	*/

	// Generate Initial Sequence Number
	uint32_t seqNum = rand();

	// Generate Request Packet
	std::vector<uint8_t> acknowledgePacket = RPacket::SerializeInstance(seqNum, 0, 0, 0, 0, std::vector<uint8_t>());

	// Begin Establishing Connection
	RPacket data;
	uint32_t packetSize;
	uint32_t bytesReceived;
	char buffer[512];

	std::string fromAddress = serverAddress;
	unsigned short fromPort = serverPort;
	bool isSuccess;
	static int ADDR_LEN = sizeof(struct sockaddr_in);
	for (;;)
	{
		// Send Request Packet
		packetSize = static_cast<uint32_t>(acknowledgePacket.size());
		try
		{
			serverSocket->SendTo(reinterpret_cast<char *>(acknowledgePacket.data()), packetSize, serverAddress, serverPort);
		}
		catch (SocketException ex)
		{
			Logger::PrintF(__FILE__, "sendto failed with error: %s\n", ex.what());
			return RUDPStream::InvalidStream();
		}

		// Begin Timing (Record Time it was Sent)
		std::chrono::high_resolution_clock::time_point timeSentToServer = std::chrono::high_resolution_clock::now();
		Logger::PrintF(__FILE__, "Sending to Server: ip<%s> port<%d>\n", serverAddress.c_str(), serverPort);
		Logger::PrintF(__FILE__, "Establishing Connection: Sending seq <%d> ack<%d>\n", seqNum, 0);

		for (;;)
		{
			// Get Incoming Packets
			bytesReceived = 512;
			isSuccess = serverSocket->ReceiveFrom(buffer, bytesReceived, fromAddress, fromPort, maxConnectionTimeOut);

			// Check if Packet is Received
			if (isSuccess)
			{	// Packet has been received!

				// Check if packet's address is from server
				if (serverAddress != fromAddress)
				{	// Packet is not from the server, ignore...
					Logger::PrintF(__FILE__, "RUDP Packet is not from Server: expected<%s> actual<%s>\n", serverAddress.c_str(), fromAddress.c_str());
					continue;
				}

				// Convert bytes to RUDP packet information
				isSuccess = data.Deserialize(reinterpret_cast<uint8_t *>(buffer));

				// Check if Packet is RUDP Packet
				if (!isSuccess)
				{	// Packet is not an RUDP Packet, ignore...
					Logger::PrintF(__FILE__, "Packet is not an RUDP Packet\n");
					continue;
				}

				// Packet is an RUDP Packet from Server, accept Packet!
				Logger::PrintF(__FILE__, "Server Message Received: ip<%s> port<%d>\n", fromAddress.c_str(), fromPort);
				break;
			}
			else
			{
				Logger::PrintF(__FILE__, "recvfrom produced error: %ld\n", WSAGetLastError());
				// Establishing connection timed-out. Return unsuccessful.
				Logger::PrintF(__FILE__, "Establishing Connection Timed Out.\n");
				return RUDPStream::InvalidStream();
			}
		}

		// Check if Acknowledge Number is your Sequence Number
		if (data.Ack() != seqNum + 1)
		{	// Bad Acknowledgement number, send again!
			Logger::PrintF(__FILE__, "Bad Acknowledgement number: expected<%d> actual<%d>\n", seqNum + 1, data.Ack());
			continue;
		}

		// Acknowledgement Successful!
		Logger::PrintF(__FILE__, "Acknowledged Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());
		break;
	}

	// Send Acknowledgement to Complete Three-Way Handshake
	seqNum += 1;
	uint32_t ackNum = data.Sequence() + 1;
	acknowledgePacket = RPacket::SerializeInstance(seqNum, ackNum, 0, 0, 0, std::vector<uint8_t>());

	// Send Acknowledged Packet
	packetSize = static_cast<uint32_t>(acknowledgePacket.size());
	try
	{
		serverSocket->SendTo(reinterpret_cast<char *>(acknowledgePacket.data()), packetSize, fromAddress, fromPort);
	}
	catch (SocketException ex)
	{
		Logger::PrintF(__FILE__, "sendto failed with error: %s\n", ex.what());
		return RUDPStream::InvalidStream();
	}


	// At this point the connection is established
	Logger::PrintF(__FILE__, "Acknowledging Connection: Sending  seq <%d> ack<%d>\n", seqNum, ackNum);

	// Next message that will be sent should be one sequence number higher
	seqNum += 1;

	return RUDPStream(serverSocket, fromAddress, fromPort, seqNum, ackNum, maxConnectionTimeOut);
	//return RUDPStream(serverSocket, serverAddress, seqNum, ackNum, maxConnectionTimeOut);
}