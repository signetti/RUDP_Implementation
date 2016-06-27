#include "stdafx.h"
#include "RUDPStream.h"
#include "RPacket.h"
#include <assert.h>

RUDPStream::RUDPStream(const SOCKET& socket, const struct sockaddr& toAddress, const uint32_t& senderSequenceNumber, const uint32_t& receiverSequenceNumber, uint32_t maxConnectionTimeOut)
	: mSocket(socket), mToAddress(toAddress), mSequenceNumber(senderSequenceNumber), mRemoteSequenceNumber(receiverSequenceNumber)
	, mMaxConnectionTimeOut(maxConnectionTimeOut), mBuffer(new char[sMaximumTransmissionUnit]) 
{
	/*
	// Before beginning the process, we must make the recvfrom non-blocking
	// This is to check on a time-out period to deliver the message again.
	DWORD NON_BLOCKING = TRUE;
	if (ioctlsocket(mSocket, FIONBIO, &NON_BLOCKING) != 0)
	{
		printf("RUDPStream failed to set non-blocking\n");
	}
	*/
}

RUDPStream::RUDPStream(const SOCKET& socket) : mSocket(socket), mToAddress() , mMaxConnectionTimeOut(0), mBuffer(new char[sMaximumTransmissionUnit])
{
	//Shutdown(SD_SEND);
}

RUDPStream::RUDPStream(const RUDPStream & other)
	: mSocket(other.mSocket), mToAddress(other.mToAddress), mMaxConnectionTimeOut(other.mMaxConnectionTimeOut), mBuffer(new char[sMaximumTransmissionUnit])
{}

RUDPStream & RUDPStream::operator=(const RUDPStream & rhs)
{
	if (this != &rhs)
	{
		mSocket = rhs.mSocket;
		mToAddress = rhs.mToAddress;
		mMaxConnectionTimeOut = rhs.mMaxConnectionTimeOut;
	}
	return *this;
}

RUDPStream::RUDPStream(RUDPStream && other)	: mSocket(std::move(other.mSocket)), mToAddress(std::move(other.mToAddress))
	, mMaxConnectionTimeOut(std::move(other.mMaxConnectionTimeOut)), mBuffer(new char[sMaximumTransmissionUnit]) {}

RUDPStream & RUDPStream::operator=(RUDPStream && rhs)
{
	if (this != &rhs)
	{
		mSocket = std::move(rhs.mSocket);
		mToAddress = std::move(rhs.mToAddress);
		mMaxConnectionTimeOut = std::move(rhs.mMaxConnectionTimeOut);
		rhs.mSocket = INVALID_SOCKET;
	}
	return *this;
}

RUDPStream::~RUDPStream()
{
	delete mBuffer;
	Close();
}


int RUDPStream::Send(const char * message)
{
	return Send(message, (int)strlen(message) + 1);
}

int RUDPStream::Send(const std::string& message)
{
	return Send(message.c_str(), static_cast<int>(message.length()) + 1);
}

int RUDPStream::Send(const char * data, uint32_t sizeOfData)
{
	return Send(reinterpret_cast<const uint8_t *>(data), sizeOfData);
}
int RUDPStream::Send(const uint8_t * data, uint32_t sizeOfData)
{
	int totalBytesSent = 0;
	RPacket packet;
	
	// ======= Send the Data in Packets =====

	// Convert Size to Integer Data
	int32_t currentSizeOfData = static_cast<int32_t>(sizeOfData);
	if (currentSizeOfData < 0)
	{	// Size overflowed, size is too large, don't even send
		return -1;
	}

	/*
	// Initialize "Sliding Window" characteristics
	uint32_t lastRemoteSequenceAcknowledged;

	// Initialize Acknowledgement characteristics
	int bytesReceived;
	bool isSuccess;
	*/
	ackbitfield_t acknowledgedBitfield;

	// Check if Fragmentation is Required
	if (sizeOfData <= sMaximumDataSize)
	{	// No Fragmentation Necessary, Simply send a single packet!
		
		// Create Single-Data Packet
		packet.Initialize(mSequenceNumber, mRemoteSequenceNumber, 0U, data, sizeOfData);
		std::vector<uint8_t> packetData = packet.Serialize();

		// Send Single-Data Packet
		totalBytesSent = sendto(mSocket, reinterpret_cast<char *>(packetData.data()), static_cast<int>(packetData.size()), 0, &mToAddress, SOCK_ADDR_SIZE);
		if (totalBytesSent < 0)
		{
			printf("sendto failed with error: %d\n", WSAGetLastError());
			printf("Sending: socket<%d> address<%d.%d.%d.%d.%d.%d.%d.%d.%d.%d>\n", static_cast<int>(mSocket)
				, (uint8_t)(mToAddress.sa_data[0]), (uint8_t)(mToAddress.sa_data[1]), (uint8_t)(mToAddress.sa_data[2]), (uint8_t)(mToAddress.sa_data[3]), (uint8_t)(mToAddress.sa_data[4])
				, (uint8_t)(mToAddress.sa_data[5]), (uint8_t)(mToAddress.sa_data[6]), (uint8_t)(mToAddress.sa_data[7]), (uint8_t)(mToAddress.sa_data[8]), (uint8_t)(mToAddress.sa_data[9]));

			return -1;
		}

		// Update Sequence Number
		mSequenceNumber++;
		
		// DEBUG: For Debug Purposes only
		printf("message sent [%d bytes]:\t(seq:%d ack:%d ack_bit:%d msg_size:%d)\n"
			, totalBytesSent, packet.Sequence(), packet.Ack(), packet.AckBitfield().bitfield, sizeOfData);
	}
	else
	{	// Fragmentation Required!

		// Calculate number and size of Fragments (using Ceiling Function)
		int numberOfFragments = ((sizeOfData - 1) / sMaximumDataSize) + 1;
		int sizeOfPacketToSend = ((sizeOfData - 1) / numberOfFragments) + 1;
		
		const uint8_t * currentDataLocation = data;
		int numOfBytesToSend;
		int bytesSent;

		// Send with fragment data
		for (int fragment = 0; fragment < numberOfFragments; ++fragment)
		{
			// Get the Minimum Between the current bytes left and the largest sized packet to send
			numOfBytesToSend = (currentSizeOfData < sizeOfPacketToSend) ? currentSizeOfData : sizeOfPacketToSend;

			// Create Packet Data (of appropriate size)
			packet.Initialize(mSequenceNumber, mRemoteSequenceNumber, acknowledgedBitfield, currentDataLocation, numOfBytesToSend);
			std::vector<uint8_t> packetData = packet.Serialize();

			// Send the Packet Data
			bytesSent = sendto(mSocket, reinterpret_cast<char *>(packetData.data()), static_cast<int>(packetData.size()), 0, &mToAddress, SOCK_ADDR_SIZE);
			if (bytesSent < 0)
			{
				printf("sendto failed with error: %d\n", WSAGetLastError());
				return -1;
			}
			else if (bytesSent == 0)
			{
				return 0;
			}
			assert(bytesSent == static_cast<int>(packetData.size()));

			// Update Sequence Number
			mSequenceNumber++;

			// Update bytes left to send
			currentDataLocation += numOfBytesToSend;
			currentSizeOfData -= numOfBytesToSend;
			totalBytesSent += bytesSent;

			// DEBUG: For Debug Purposes only
			printf("message sent [%d bytes]:\t(seq:%d ack:%d ack_bit:%d msg_size:%d)\n"
				, bytesSent, packet.Sequence(), packet.Ack(), packet.AckBitfield().bitfield, numOfBytesToSend);

			/*
			// Check for Acknowledgement (once per send)
			{
				// Get Incoming Packets
				bytesReceived = recvfrom(mSocket, mBuffer, sBufferSize, 0, NULL, NULL);

				// Check if Packet is Received
				if (bytesReceived > 0)
				{	// Packet has been received!

					// Convert bytes to RUDP packet information
					isSuccess = packet.Deserialize(reinterpret_cast<uint8_t *>(mBuffer));

					// Check if Packet is RUDP Packet
					if (!isSuccess)
					{	// Packet is not an RUDP Packet, ignore...
						printf("Packet is not an RUDP Packet\n");
					}
					else
					{	// Packet is an RUDP Packet from Server, Check Data

					}
				}
				else if (WSAGetLastError() != WSAEWOULDBLOCK)
				{	// Error Reported!
					printf("recvfrom produced error: %ld\n", WSAGetLastError());
				}
			}*/
		}

		assert(currentSizeOfData <= 0);	// Number of data transmitted should be everything!
	}

	/*
	// ==== Receive Acknowledgement of Received Packets ====

	// Set now as the last connection time
	mLastConnectionTime = std::chrono::high_resolution_clock::now();

	for (;;)
	{
		// Await the Reads
		for (;;)
		{
			// Get Incoming Packets
			bytesReceived = recvfrom(mSocket, mBuffer, sMaximumTransmissionUnit, 0, NULL, NULL);

			// Check if Packet is Received
			if (bytesReceived > 0)
			{	// Packet has been received!
				// Convert bytes to RUDP packet information
				isSuccess = packet.Deserialize(reinterpret_cast<uint8_t *>(mBuffer));

				// Check if Packet is RUDP Packet
				if (!isSuccess)
				{	// Packet is not an RUDP Packet, ignore...
					printf("Packet is not an RUDP Packet\n");
					continue;
				}

				// Packet is an RUDP Packet from Server, Check Data
				break;
			}
			else if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				printf("recvfrom produced error: %ld\n", WSAGetLastError());
			}

			// Sleep for a very short period, so as not to take up CPU power
			Sleep(1);

			// Check if Request timed-out
			if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - mLastConnectionTime).count() >= mMaxConnectionTimeOut)
			{
				// Establishing connection timed-out. Return unsuccessful.
				printf("send acknowledgement timed out.");
				break;
			}
		}
		
		// Check if Acknowledge Number is your Sequence Number
		//if (data.Ack() != seqNum + 1)
		//{	// Bad Acknowledgement number, send again!
		//	printf("Bad Acknowledgement number: expected<%d> actual<%d>\n", seqNum + 1, data.Ack());
		//	continue;
		//}
	}

	// Record Last time receiving from remote client
	mLastConnectionTime = std::chrono::high_resolution_clock::now();
	*/
	// TODO: Some Acknowledgement Should Happen here...

	// Return Results
	return totalBytesSent;
}


int RUDPStream::Receive(char * OutBuffer, uint32_t sizeOfBuffer)
{
	int bytesReceived;
	uint32_t size = sizeOfBuffer + 100;		// size+100 extra bytes for the header space
	uint8_t * buffer = new uint8_t[size];

	// Receive bytes
	bytesReceived = recvfrom(mSocket, reinterpret_cast<char*>(buffer), size, 0, NULL, NULL);
	if (bytesReceived == SOCKET_ERROR)
	{
		printf("recvfrom failed with error: %d\n", WSAGetLastError());
		return SOCKET_ERROR;
	}

	// Convert Bytes into RUDP Packet
	RPacket packet;
	packet.Deserialize(buffer);

	// Check it data is not an RUDP Packet
	if (packet.IsBadPacket())
	{	// Not an RUDP Packet
		printf("Not an RUDP Packet: expected id<%d>, actual id<%d>\n", packet.RUDP_ID, packet.Id());
		return SOCKET_ERROR;
	}
	else
	{	// For Debug Purposes only
		printf("message received [%d bytes]:\t(seq:%d ack:%d ack_bit:%d)\n", bytesReceived, packet.Sequence(), packet.Ack(), packet.AckBitfield().bitfield);
	}

	// TODO: Perform Sequence Checks of some sort...

	// Return results (Copy only the data into the OutBuffer)
	auto data = packet.Buffer();
	memcpy_s(OutBuffer, sizeOfBuffer, data.data(), data.size());
	return static_cast<int>(data.size());
}

bool RUDPStream::Shutdown(int how)
{
	int result;

	// shutdown the sending connection
	result = shutdown(mSocket, how);
	if (result == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		return false;
	}
	else return true;
}

void RUDPStream::Close()
{
	// Close Socket
	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
	}
}

bool RUDPStream::IsOpen()
{
	return (mSocket != INVALID_SOCKET);
}
