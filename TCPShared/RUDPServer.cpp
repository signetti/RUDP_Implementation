#include "stdafx.h"
#include "RUDPServer.h"
#include "RUDPStream.h"
#include "RPacket.h"
#include "StopWatch.h"

#include <sstream>

RUDPServer::RUDPServer(std::uint32_t listenPort, std::uint32_t maxConnectionTimeOut) : mListenSocket(INVALID_SOCKET), mInfo(NULL)
	, mPort(IntToString(listenPort)), mClientSocket(INVALID_SOCKET), mAvailablePort(listenPort + 1), mMaxConnectionTimeOut(maxConnectionTimeOut)
	, mAcknowledgeTable(), mAcceptedClients()
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
	bool success;

	// Create Listening Socket (if not created)
	success = CreateSocket();
	if (!success) return false;

	// Bind Listening Socket to port
	success = Bind();
	if (!success) return false;

	// Listen in on port
	success = Listen();
	if (!success) return false;
	else return true;
}

bool RUDPServer::CreateSocket()
{
	// Check if listening socket is already created
	if (mInfo != NULL && mListenSocket != INVALID_SOCKET)
	{
		return true;
	}

	struct addrinfo hints;
	int result;

	// Check that the address information is not retrieved
	if (mInfo == NULL)
	{
		// Get Address Information from this computer
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		result = getaddrinfo(NULL, mPort.c_str(), &hints, &mInfo);
		if (result != 0)
		{
			printf("getaddrinfo failed with error: %d\n", result);
			mInfo = NULL;
			return false;
		}
	}

	// Create Listening Socket
	mListenSocket = socket(mInfo->ai_family, mInfo->ai_socktype, mInfo->ai_protocol);
	if (mListenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		return false;
	}
	else return true;
}

bool RUDPServer::Bind()
{
	int result;

	// Make sure that the listening socket has been created
	if (mInfo == NULL || mListenSocket == INVALID_SOCKET)
	{
		return false;
	}

	// Bind the socket to the computer's port
	result = bind(mListenSocket, mInfo->ai_addr, (int)mInfo->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(mListenSocket);
		return false;
	}
	else return true;
}

std::string RUDPServer::IntToString(uint32_t number)
{
	std::stringstream message;
	message << number;
	return message.str();
}

bool RUDPServer::CreateNewSocket(SOCKET& sock, std::string port)
{
	// ============ Create Socket ============
	struct addrinfo* info;
	struct addrinfo hints;
	int result;

	// Check that the address information is not retrieved
	// Get Address Information from this computer
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	result = getaddrinfo(NULL, port.c_str(), &hints, &info);
	if (result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", result);
		return false;
	}

	// Create Socket
	sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (sock == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(info);
		return false;
	}

	// Bind the socket to the computer's port
	result = bind(sock, info->ai_addr, (int)info->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(sock);
		freeaddrinfo(info);
		return false;
	}

	// Make Socket Non-Blocking
	DWORD blocking = 0;
	if (ioctlsocket(sock, FIONBIO, &blocking) != 0)
	{
		printf("failed to set blocking\n");
		assert(0);
		return false;
	}

	// New Socket Ready! Clean up...
	freeaddrinfo(info);

	return true;
}

bool RUDPServer::Listen()
{
	static int32_t ADDR_LEN = sizeof(struct sockaddr_in);

	int bytesReceived;
	int bytesSent;
	char buffer[512];
	struct sockaddr fromAddress;
	uint32_t seqNum;
	uint32_t ackNum;
	RPacket data;
	bool isSuccess;

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
	DWORD nonBlocking = 1;
	if (ioctlsocket(mListenSocket, FIONBIO, &nonBlocking) != 0)
	{
		printf("failed to set non-blocking\n");
		return false;
	}

	// TODO: Find reason why this code does not work across computers...
	/*
	// Find and Set Open Client Port
	uint32_t MAX_PORT_CHECK;
	for (MAX_PORT_CHECK = mAvailablePort + 9, isSuccess = false; !isSuccess && mAvailablePort <= MAX_PORT_CHECK; ++mAvailablePort)
	{
		isSuccess = CreateNewSocket(mClientSocket, IntToString(mAvailablePort));
	}

	if (mAvailablePort > MAX_PORT_CHECK)
	{
		printf("failed to open new port for incoming client\n");
		return false;
	}*/
	mClientSocket = mListenSocket;

	// Listen for any RUDP Packets from clients

	// Listen for incoming message
	for (;;)
	{
		// Sleep for a very short period, so as not to take up CPU power
		Sleep(1);

		// Get Incoming Packets
		bytesReceived = recvfrom(mListenSocket, buffer, 512, 0, &fromAddress, &ADDR_LEN);

		// Check if packet is received
		isSuccess = (bytesReceived > 0);
		if (isSuccess)
		{	// Packet has been received!

			// Convert bytes to RUDP packet information
			isSuccess = data.Deserialize(reinterpret_cast<uint8_t *>(buffer));

			if (!isSuccess)
			{	// Packet is not an RUDP Packet
				printf("Received non-RUDP Packet: id<%X>\n", data.Id());
			}
		}
		else if(WSAGetLastError() != WSAEWOULDBLOCK)
		{
			printf("recvfrom produced error: %ld\n", WSAGetLastError());
		}
			
		// Check if RUDP packet is received
		if(isSuccess)
		{	// Valid RUDP Packet received!
			// Check if Client's message is for acknowledging server's request or requesting connection

			// Look Up Acknowledgement Table for Client with 
			size_t index;
			for (index = 0; index < mAcknowledgeTable.size(); ++index)
			{
				auto& client = mAcknowledgeTable[index];

				// Check if client address matches packet's from-address
				if (strcmp(client.addr.sa_data, fromAddress.sa_data) == 0)
				{
					break;
				}
			}

			// Check if Client has been Acknowledged before
			if (index < mAcknowledgeTable.size())
			{	// Client has been Acknowledged before...

				// Check if Acknowledgement is correct
				if (data.Ack() == mAcknowledgeTable[index].seqNumSent + 1U)
				{	// Successful Acknowledgement from Client!
					printf("Acknowledged Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());

					// Make Socket Blocking
					DWORD blocking = 0;
					if (ioctlsocket(mClientSocket, FIONBIO, &blocking) != 0)
					{
						printf("failed to set blocking\n");
						assert(0);
						return false;
					}

					// Assign new Socket to Client
					auto& client = mAcknowledgeTable[index];
					client.sock = mClientSocket;

					// Add to Accepted Clients Table
					mAcceptedClients.emplace_back(client);

					// Remove from Acknowledged Clients Table
					mAcknowledgeTable.erase(mAcknowledgeTable.begin() + index);

					// Connection Established
					printf("Connection Established!\n");
					return true;
				}
				else
				{	// Client has a bad acknowledgent number
					printf("Received Bad Acknowledgement Number: seq<%d> ack<%d>\n", data.Sequence(), data.Ack());
				}
			}
			else
			{	// New Client Requesting Connection!
				printf("New Client found!\n");
				printf("Establishing Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());

				seqNum = rand();
				ackNum = data.Sequence() + 1U;

				std::vector<uint8_t> acknowledgePacket = RPacket::SerializeInstance(seqNum, ackNum, 0, std::vector<uint8_t>());

				bytesSent = sendto(mClientSocket, reinterpret_cast<char *>(acknowledgePacket.data()), static_cast<int>(acknowledgePacket.size()), 0, &fromAddress, ADDR_LEN);
				if (bytesSent < 0)
				{
					printf("sendto failed with error: %d\n", WSAGetLastError());
					return false;
				}

				// Store acknowledgement information in table
				printf("Establishing Connection: Sending  seq <%d> ack<%d>\n", seqNum, ackNum);
				mAcknowledgeTable.emplace_back(PendingClientsT(fromAddress, seqNum, ackNum, std::chrono::high_resolution_clock::now()));

			}
		}
		else
		{	// RUDP Packet not received

			// Get Current Time to check time-outs
			std::chrono::high_resolution_clock::time_point checkTime = std::chrono::high_resolution_clock::now();

			// Check if any requests timed out
			for (auto& ackInfo : mAcknowledgeTable)
			{
				if (std::chrono::duration_cast<std::chrono::milliseconds>(checkTime - ackInfo.time_stamp).count() >= mMaxConnectionTimeOut)
				{	// Connection timed-out for this acknowledgement

					// Compute the port (for Debug purposes)
					int32_t port1 = (*reinterpret_cast<uint16_t*>(ackInfo.addr.sa_data));
					int32_t port2 = port1 & 0x0000FFFF;
					port1 = (port2 << 8) | (port2 >> 8);
					port1 &= 0x0000FFFF;

					// Re-send the acknowledgement of connection
					printf("Connection timed out for Client <%d.%d.%d.%d : %d or %d or %d-%d >\n"
						, (uint8_t)(ackInfo.addr.sa_data[2]), (uint8_t)(ackInfo.addr.sa_data[3])
						, (uint8_t)(ackInfo.addr.sa_data[4]), (uint8_t)(ackInfo.addr.sa_data[5])
						, port1, port2, (uint8_t)(ackInfo.addr.sa_data[0]), (uint8_t)(ackInfo.addr.sa_data[1]));
					//, (uint8_t)(ackInfo.addr.sa_data[6]), (uint8_t)(ackInfo.addr.sa_data[7]), (uint8_t)(ackInfo.addr.sa_data[8]), (uint8_t)(ackInfo.addr.sa_data[9]));

					// Create the Acknowledgement Packet
					std::vector<uint8_t> acknowledgePacket = RPacket::SerializeInstance(ackInfo.seqNumSent, ackInfo.ackNumRecvd, 0, std::vector<uint8_t>());

					// Send the Acknowledgement Packet
					bytesSent = sendto(mClientSocket, reinterpret_cast<char *>(acknowledgePacket.data()), static_cast<int>(acknowledgePacket.size()), 0, &ackInfo.addr, ADDR_LEN);
					if (bytesSent < 0)
					{
						printf("sendto failed with error: %d\n", WSAGetLastError());
						assert(false);
					}

					// Reset Time Stamp
					printf("Establishing Connection: Sending seq <%d> ack<%d>\n", ackInfo.seqNumSent, ackInfo.ackNumRecvd);
					ackInfo.time_stamp = checkTime;
				}
			}
		}
	}
}

RUDPStream * RUDPServer::Accept()
{
	// No Accepted Clients
	if (mAcceptedClients.empty())
	{
		return new RUDPStream(INVALID_SOCKET);
	}

	auto& client = mAcceptedClients.back();
	/*
	if (!CreateRandomSocket(client.sock, mPort))
	{
		assert(0);
	}*/
	/*
	// Before returning, make it listen
	DWORD blocking = 0;
	if (ioctlsocket(clientSocket, FIONBIO, &blocking) != 0)
	{
	printf("failed to set non-blocking\n");
	return false;
	}*/

	auto& clientSocket = client.sock;
	auto& clientAddress = client.addr;



	/*
	// Get Address
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	// Resolve the server address and port
	struct addrinfo* info;
	int result;

	result = getaddrinfo(clientAddress.sa_data, mPort.c_str(), &hints, &info);
	if (result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", result);
		return RUDPStream(INVALID_SOCKET);
	}

	// Create Socket
	SOCKET sock;
	sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (sock == INVALID_SOCKET)
	{
		printf("Could not create client socket.\n");
		return RUDPStream(INVALID_SOCKET);
	}*/

	// Remove the created client socket from the accepted client socket
	mAcceptedClients.pop_back();
	RUDPStream * newClientStream = new RUDPStream(clientSocket, clientAddress, client.seqNumSent, client.ackNumRecvd, mMaxConnectionTimeOut);
	mClients.push_back(newClientStream);
	return newClientStream;
}

void RUDPServer::Close()
{
	// Clear address information
	if (mInfo == NULL)
	{
		freeaddrinfo(mInfo);
		mInfo = NULL;
	}

	// Close listening socket
	if (mListenSocket != INVALID_SOCKET)
	{
		closesocket(mListenSocket);
		mListenSocket = INVALID_SOCKET;
	}

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