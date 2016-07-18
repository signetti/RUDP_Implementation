#include "stdafx.h"
#include "RUDPServer.h"
#include "RUDPStream.h"
#include "RPacket.h"
#include "StopWatch.h"

#include "UDPSocket.h"
#include "SocketException.h"

#include "Logger.h"

#include <sstream>

RUDPServer::RUDPServer(std::uint16_t listenPort, std::uint32_t maxConnectionTimeOut)
	: mListenSocket(std::make_shared<UDPSocket>(listenPort)), mAvailablePort(listenPort + 1), mMaxConnectionTimeOut(maxConnectionTimeOut), mAcknowledgeTable()
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

	uint32_t bytesReceived;
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
		Logger::PrintF("Testing port %d ... ", mAvailablePort);
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
		Logger::PrintF("failed to open new port for incoming client\n");
		return nullptr;
	}
	else
	{
		Logger::PrintF("Port Available: %d\n", mAvailablePort);
	}

	*/

	// Listen for any RUDP Packets from clients
	std::string clientAddress;
	unsigned short clientPort = 0;

	bool isClientFound = false;

	// Listen for incoming message
	for (;;)
	{
		// Sleep for a very short period, so as not to take up CPU power
		Sleep(1);

		if (!isClientFound)
		{
			// Get Incoming Packets
			bytesReceived = 512;
			isSuccess = mListenSocket->ReceiveFrom(buffer, bytesReceived, clientAddress, clientPort, mMaxConnectionTimeOut);

			// Check if packet is received
			if (isSuccess)
			{	// Packet has been received!

				// Convert bytes to RUDP packet information
				isSuccess = data.Deserialize(reinterpret_cast<uint8_t *>(buffer));

				if (!isSuccess)
				{	// Packet is not an RUDP Packet
					Logger::PrintF("Received non-RUDP Packet on Listening Port %d: id<%X>\n", mListenSocket->GetLocalPort(), data.Id());
				}
			}
		}
		else
		{
			Logger::PrintF("Ignore listen\n");
			isSuccess = false;
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
				Logger::PrintF("New Client found! ip<%s> port<%d>\n", clientAddress.c_str(), clientPort);
				isClientFound = true;

				Logger::PrintF("Establishing Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());

				// Create RPacket to send...
				seqNum = rand();
				ackNum = data.Sequence() + 1U;
				std::vector<uint8_t> acknowledgePacket = RPacket::SerializeInstance(seqNum, ackNum, 0, 0, 0, std::vector<uint8_t>());

				// Create new socket to communicate with client
				std::shared_ptr<UDPSocket> clientSocket(std::make_shared<UDPSocket>(mAvailablePort));
				++mAvailablePort;

				try
				{
					clientSocket->SendTo(acknowledgePacket.data(), static_cast<int>(acknowledgePacket.size()), clientAddress, clientPort);
				}
				catch (SocketException ex)
				{
					Logger::PrintF("sendto failed with error: %s\n", ex.what());
				}

				// Store acknowledgement information in table
				Logger::PrintF("Establishing Connection: Sending  seq <%d> ack<%d>\n", seqNum, ackNum);
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
				bytesReceived = 512;
				isSuccess = client.socket->ReceiveFrom(buffer, bytesReceived, clientAddress, clientPort, mMaxConnectionTimeOut);

				// Check if packet is received
				isSuccess = (bytesReceived > 0);
				if (isSuccess)
				{	// Packet has been received!

					// Convert bytes to RUDP packet information
					isSuccess = data.Deserialize(reinterpret_cast<uint8_t *>(buffer));

					if (!isSuccess)
					{	// Packet is not an RUDP Packet
						Logger::PrintF("Received non-RUDP Packet on Port %d: id<%X>\n", client.socket->GetLocalPort(), data.Id());
					}
				}

				if(isSuccess)
				{	// Client has been Acknowledged before...

					// Check if Acknowledgement is correct
					if (data.Ack() == client.seqNumSent + 1U)
					{	// Successful Acknowledgement from Client!
						Logger::PrintF("Acknowledged Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());

						// Create RUDP Stream
						RUDPStream * newClientStream = new RUDPStream(client.socket, client.address, client.port, client.seqNumSent + 1, client.ackNumRecvd + 1, mMaxConnectionTimeOut);

						// Remove from Acknowledged Clients Table
						removeIndices.push_back(index);

						// Connection Established
						Logger::PrintF("Connection Established!\n");

						// Add to the list of Clients
						mClients.push_back(newClientStream);

						// Return the client
						return newClientStream;
					}
					else
					{	// Client has a bad acknowledgent number
						Logger::PrintF("Received Bad Acknowledgement Number: seq<%d> ack<%d>\n", data.Sequence(), data.Ack());
					}
				}
				else
				{	// Connection timed-out for this acknowledgement
					Logger::PrintF("Connection Timed-Out for Client %s:%d\n", client.address.c_str(), client.port);

					// Remove from Acknowledged Clients Table
					removeIndices.push_back(index);
				}
			}

			// Remove from list
			index = 0;
			for (auto& removeIndex : removeIndices)
			{
				mAcknowledgeTable.erase(mAcknowledgeTable.begin() + removeIndex - index);
				++index;	// Record offset
			}

			isClientFound = (mAcknowledgeTable.size() != 0);
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