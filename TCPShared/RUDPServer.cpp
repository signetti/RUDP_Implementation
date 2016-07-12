#include "stdafx.h"
#include "RUDPServer.h"
#include "RUDPStream.h"
#include "RPacket.h"
#include "StopWatch.h"

#include <sstream>

RUDPServer::RUDPServer(std::uint16_t listenPort, std::uint32_t maxConnectionTimeOut)
	: mListenSocket(make_shared<UDPSocket>(listenPort)), mAvailablePort(listenPort + 1000), mMaxConnectionTimeOut(maxConnectionTimeOut), mAcknowledgeTable()
{
	// Start-Up the WSA Library
	WSAManager::StartUp();
}


RUDPServer::~RUDPServer()
{
	Close();
}

bool RUDPServer::Open()
{
	return true;
}

std::string RUDPServer::IntToString(uint32_t number)
{
	std::stringstream message;
	message << number;
	return message.str();
}

RUDPStream * RUDPServer::Accept()
{
	static int32_t ADDR_LEN = sizeof(struct sockaddr_in);

	int bytesReceived;
	char buffer[512];
	uint32_t seqNum;
	uint32_t ackNum;
	RPacket data;
	bool isSuccess;

	// Available Open Socket for incoming clients
	//shared_ptr<UDPSocket> clientSocket = shared_ptr<UDPSocket>(nullptr);
	//shared_ptr<UDPSocket> clientSocket = make_shared<UDPSocket>(mListenSocket);
	//mAvailablePort += 1;
	//shared_ptr<UDPSocket> clientSocket(make_shared<UDPSocket>(mAvailablePort));

	/** Perform Three-way Hand-shaking
	*	This works as follows
	*	 - Client sends message with SeqNum X
	*	 - Server receives message and returns message with SeqNum Y and AckNum X+1
	*	 - Client receives, checks AckNum for X+1, and returns message with AckNum Y+1
	*	 - Server receives message, checks AckNum for Y+1, and
	*	Returning the Num+1 tells the receiving end that the number was recognized.
	*/
	// TODO: Find reason why this code does not work across computers...

/*
	// Find and Set Open Client Port
	uint32_t MAX_PORT_CHECK;
	++mAvailablePort;
	for (MAX_PORT_CHECK = mAvailablePort + 9, isSuccess = false; !isSuccess && mAvailablePort <= MAX_PORT_CHECK; ++mAvailablePort)
	{
		printf("Testing port %d ... ", mAvailablePort);
		try
		{
			clientSocket = make_shared<UDPSocket>(mAvailablePort);
			isSuccess = true;
			break;
		}
		catch (SocketException e) {}
	}

	if (mAvailablePort > MAX_PORT_CHECK)
	{
		printf("failed to open new port for incoming client\n");
		return nullptr;
	}
	else
	{
		printf("Port Available: %d\n", mAvailablePort);
	}

	*/

	// Listen for any RUDP Packets from clients
	std::string clientAddress;
	unsigned short clientPort;


	// Listen for incoming message
	for (;;)
	{
		// Sleep for a very short period, so as not to take up CPU power
		Sleep(1);

		// Get Incoming Packets
		bytesReceived = mListenSocket->recvFrom(buffer, 512, clientAddress, clientPort, 500/*mMaxConnectionTimeOut*/);

		// Check if packet is received
		isSuccess = (bytesReceived > 0);
		if (isSuccess)
		{	// Packet has been received!

			// Convert bytes to RUDP packet information
			isSuccess = data.Deserialize(reinterpret_cast<uint8_t *>(buffer));

			if (!isSuccess)
			{	// Packet is not an RUDP Packet
				printf("Received non-RUDP Packet on Listening Port %d: id<%X>\n", mListenSocket->getLocalPort(), data.Id());
			}
		}

		// Check if RUDP packet is received
		if (isSuccess)
		{	// Valid RUDP Packet received!
			// Check if Client's message is for acknowledging server's request or requesting connection

			// Look Up Acknowledgement Table for Client with 
			size_t index;
			for (index = 0; index < mAcknowledgeTable.size(); ++index)
			{
				auto& client = mAcknowledgeTable[index];

				// Check if client address matches packet's from-address
				if (client.address == clientAddress)
				{
					break;
				}
			}

			// Check if Client has been Acknowledged before (ignore message if that's the case)
			if (index >= mAcknowledgeTable.size())
			{	// New Client Requesting Connection!
				printf("New Client found! ip<%s> port<%d>\n", clientAddress.c_str(), clientPort);

				printf("Establishing Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());

				// Create RPacket to send...
				seqNum = rand();
				ackNum = data.Sequence() + 1U;
				std::vector<uint8_t> acknowledgePacket = RPacket::SerializeInstance(seqNum, ackNum, 0, 0, 0, std::vector<uint8_t>());

				// Create new socket to communicate with client
				//shared_ptr<UDPSocket> clientSocket(make_shared<UDPSocket>(mAvailablePort));
				//++mAvailablePort;
				shared_ptr<UDPSocket> clientSocket(make_shared<UDPSocket>());

				try
				{
					clientSocket->sendTo(acknowledgePacket.data(), static_cast<int>(acknowledgePacket.size()), clientAddress, clientPort);
				}
				catch (SocketException ex)
				{
					printf("sendto failed with error: %s\n", ex.what());
				}

				// Store acknowledgement information in table
				printf("Establishing Connection: Sending  seq <%d> ack<%d>\n", seqNum, ackNum);
				mAcknowledgeTable.emplace_back(PendingClientsT(clientSocket, clientAddress, clientPort, seqNum, ackNum, std::chrono::high_resolution_clock::now()));
			}
		}
		else
		{	// RUDP Packet not received

			// Get Current Time to check time-outs
			//std::chrono::high_resolution_clock::time_point checkTime = std::chrono::high_resolution_clock::now();

			// Check if any requests timed out
			std::vector<int> removeIndices;
			int index = 0;
			for (auto& client : mAcknowledgeTable)
			{
				bytesReceived = client.socket->recvFrom(buffer, 512, clientAddress, clientPort, mMaxConnectionTimeOut);

				// Check if packet is received
				isSuccess = (bytesReceived > 0);
				if (isSuccess)
				{	// Packet has been received!

					// Convert bytes to RUDP packet information
					isSuccess = data.Deserialize(reinterpret_cast<uint8_t *>(buffer));

					if (!isSuccess)
					{	// Packet is not an RUDP Packet
						printf("Received non-RUDP Packet on Port %d: id<%X>\n", client.socket->getLocalPort(), data.Id());
					}
				}

				if(isSuccess)
				{	// Client has been Acknowledged before...

					// Check if Acknowledgement is correct
					if (data.Ack() == client.seqNumSent + 1U)
					{	// Successful Acknowledgement from Client!
						printf("Acknowledged Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());

						// Create RUDP Stream
						RUDPStream * newClientStream = new RUDPStream(client.socket, client.address, client.port, client.seqNumSent + 1, client.ackNumRecvd + 1, mMaxConnectionTimeOut);

						// Remove from Acknowledged Clients Table
						removeIndices.push_back(index);

						// Connection Established
						printf("Connection Established!\n");

						// Add to the list of Clients
						mClients.push_back(newClientStream);

						// Return the client
						return newClientStream;
					}
					else
					{	// Client has a bad acknowledgent number
						printf("Received Bad Acknowledgement Number: seq<%d> ack<%d>\n", data.Sequence(), data.Ack());
					}
				}
				else
				{	// Connection timed-out for this acknowledgement
					printf("Connection Timed-Out for Client %s:%d\n", client.address.c_str(), client.port);

					// Remove from Acknowledged Clients Table
					removeIndices.push_back(index);
				}
			}

			// Remove from list
			for (auto& removeIndex : removeIndices)
			{
				mAcknowledgeTable.erase(mAcknowledgeTable.begin() + removeIndex);
			}
		}
	}
}

void RUDPServer::Close()
{
	// Close listening socket
	mListenSocket->Close();

	// Clear list of clients
	for (auto& stream : mClients)
	{
		if (stream->IsOpen())
		{
			stream->Close();
			delete stream;
		}
	}
	mClients.clear();
}