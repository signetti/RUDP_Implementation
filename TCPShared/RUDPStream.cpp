#include "stdafx.h"
#include "RUDPStream.h"
#include "RPacket.h"
#include <assert.h>

RUDPStream::RUDPStream(const SOCKET& socket, const struct sockaddr& toAddress, const uint32_t& senderSequenceNumber, const uint32_t& receiverSequenceNumber, uint32_t maxConnectionTimeOut)
	: mSocket(socket), mToAddress(toAddress), mSequenceNumber(senderSequenceNumber), mRemoteSequenceNumber(receiverSequenceNumber)
	, mMaxConnectionTimeOut(maxConnectionTimeOut), mBuffer(new uint8_t[sBufferSize])
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

RUDPStream::RUDPStream(const SOCKET& socket) : mSocket(socket), mToAddress() , mMaxConnectionTimeOut(0), mBuffer(new uint8_t[sBufferSize])
{
	//Shutdown(SD_SEND);
}

RUDPStream::RUDPStream(const RUDPStream & other)
	: mSocket(other.mSocket), mToAddress(other.mToAddress), mMaxConnectionTimeOut(other.mMaxConnectionTimeOut), mBuffer(new uint8_t[sBufferSize])
{
	memcpy_s(mBuffer, sBufferSize, other.mBuffer, sBufferSize);
}

RUDPStream & RUDPStream::operator=(const RUDPStream & rhs)
{
	if (this != &rhs)
	{
		mSocket = rhs.mSocket;
		mToAddress = rhs.mToAddress;
		mMaxConnectionTimeOut = rhs.mMaxConnectionTimeOut;
		memcpy_s(mBuffer, sBufferSize, rhs.mBuffer, sBufferSize);
	}
	return *this;
}

RUDPStream::RUDPStream(RUDPStream && other)	: mSocket(std::move(other.mSocket)), mToAddress(std::move(other.mToAddress))
	, mMaxConnectionTimeOut(std::move(other.mMaxConnectionTimeOut)), mBuffer(other.mBuffer) 
{
	other.mSocket = INVALID_SOCKET;
	other.mBuffer = nullptr;
}

RUDPStream & RUDPStream::operator=(RUDPStream && rhs)
{
	if (this != &rhs)
	{
		mSocket = std::move(rhs.mSocket);
		mToAddress = std::move(rhs.mToAddress);
		mMaxConnectionTimeOut = std::move(rhs.mMaxConnectionTimeOut);
		mBuffer = rhs.mBuffer;

		rhs.mSocket = INVALID_SOCKET;
		rhs.mBuffer = nullptr;
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

	/*
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

	// Declare this as a new message
	++mMessageId;

	// Check if Fragmentation is Required
	if (sizeOfData <= sMaximumDataSize)
	{	// No Fragmentation Necessary, Simply send a single packet!
		
		// Create Single-Data Packet
		packet.Initialize(mSequenceNumber, mRemoteSequenceNumber, 0U, mMessageId, 1U, data, sizeOfData);
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


		// Initialize "Sliding Window" characteristics
		ackbitfield_t sentPacketsAckBitfield;								// Tracks the Received Packets from the Receiver and sets the AckBitfield accordingly
		ackbitfield_t recvdPacketsAckBitfield;								// Tracks the Sent Packets Acknowledged by the Receiver
		SlidingWindow window;												// Sliding Window used to track the state of the Sent Packets
		PacketFrame currentFrame;
		PacketFrame poppedFrame;
		std::vector<PacketFrame> packetsLost;

		// Initialize Acknowledgement characteristics
		int bytesReceived;
		bool isSuccess;

		// Send with fragment data
		for (int fragment = 0; fragment < numberOfFragments; ++fragment)
		{
			// Get the Minimum Between the current bytes left and the largest sized packet to send
			numOfBytesToSend = (currentSizeOfData < sizeOfPacketToSend) ? currentSizeOfData : sizeOfPacketToSend;

			// Create Packet Data (of appropriate size)
			packet.Initialize(mSequenceNumber, mRemoteSequenceNumber, recvdPacketsAckBitfield, mMessageId, numberOfFragments, currentDataLocation, numOfBytesToSend);
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

			// Create Current Packet Frame
			currentFrame.Reassign(mSequenceNumber, currentDataLocation, numOfBytesToSend);

			// Update Sliding Window
			isSuccess = window.PushBack(currentFrame, &poppedFrame);

			// Check if Sliding Window pushed out a value
			if (!isSuccess)
			{	// Handle the packet that did not go through
				if (sentPacketsAckBitfield.bit31 == false)
				{	// Packet was never acknowledged, save result to be handled later
					packetsLost.push_back(poppedFrame);
				}
			}

			// Update Acknowledgement Bits (Shift bits same as Sliding Window)
			sentPacketsAckBitfield.bitfield <<= 1;

			// Update Sequence Number
			mSequenceNumber++;

			// Update bytes left to send
			currentDataLocation += numOfBytesToSend;
			currentSizeOfData -= numOfBytesToSend;
			totalBytesSent += bytesSent;

			// DEBUG: For Debug Purposes only
			printf("message sent [%d bytes]:\t(seq:%d ack:%d ack_bit:%d msg_size:%d)\n"
				, bytesSent, packet.Sequence(), packet.Ack(), packet.AckBitfield().bitfield, numOfBytesToSend);

			// Check for any Acknowledgements Received
			do
			{
				// Get Incoming Packets
				bytesReceived = recvfrom(mSocket, reinterpret_cast<char *>(mBuffer), sBufferSize, 0, NULL, NULL);

				// Check if Packet is Received
				if (bytesReceived > 0)
				{	// Packet has been received!

					// Convert bytes to RUDP packet information
					isSuccess = packet.Deserialize(mBuffer);

					// Check if Packet is RUDP Packet
					if (!isSuccess)
					{	// Packet is not an RUDP Packet, ignore...
						printf("Packet is not an RUDP Packet\n");
					}
					else if(packet.MessageId() == mMessageId)
					{	// Packet is an RUDP Packet from Server, Check Data

						// =========== Handling Acknowledged Packets ===========

						// 2. Handle Packet's Sequence Number
						// Check if the Packet received is more recent
						if (IsSeq2MoreRecent(mRemoteSequenceNumber, packet.Sequence()))
						{	// Update last ack received to this packet's sequence
							int32_t newRemoteSequenceOffset = DiffFromSeq1ToSeq2(packet.Sequence(), mRemoteSequenceNumber);
							assert(newRemoteSequenceOffset >= 0);	// Assert Seq2 is in fact more recent

							mRemoteSequenceNumber = packet.Sequence();						// Update to latest remote sequence number
							recvdPacketsAckBitfield.bitfield <<= newRemoteSequenceOffset;	// Shift to match latest remote sequence number
						}
						else
						{	// Set Received Packet's Ackbit
							int32_t ackbitToSetPlusOne = DiffFromSeq1ToSeq2(mRemoteSequenceNumber,packet.Sequence());
							assert(ackbitToSetPlusOne >= 0);	// Assert Seq1 is in fact more recent

							if (ackbitToSetPlusOne > 0 && ackbitToSetPlusOne < RPacket::NumberOfAcksPerPacket)
							{	// Bit to Set is within [0,31]
								recvdPacketsAckBitfield.Set(ackbitToSetPlusOne - 1, true);
							}
							else
							{	// DEBUG: For Debug Purposes Only
								printf("Ack Received, but not able to Set value...\n");
							}
						}

						// 3. Handle Packet's Ack Number and Ack-Bitfield
						// Set the sentPacket's Ack Bitfield according to the Ack and AckBitfield received
						int sentAckBitfieldOffset = DiffFromSeq1ToSeq2(mSequenceNumber, packet.Ack());
						if (sentAckBitfieldOffset > 0 && sentAckBitfieldOffset < RPacket::NumberOfAcksPerPacket)
						{	// Bits to Set is within [0,31]
							sentAckBitfieldOffset = sentAckBitfieldOffset - 1;	// Get Offset range in [0,31]
							sentPacketsAckBitfield.Set(sentAckBitfieldOffset, false);										// Set the received Ack Number
							sentPacketsAckBitfield.bitfield |= (packet.AckBitfield().bitfield << sentAckBitfieldOffset);	// Set All Acks in received AckBitfield
						}
					}
					else
					{
						printf("Receiving invalid message id: expected<%d>, actual<%d>", mMessageId, packet.MessageId());
					}
				}
				else if (WSAGetLastError() != WSAEWOULDBLOCK)
				{	// Error Reported! Exit Loop
					printf("recvfrom produced error: %ld\n", WSAGetLastError());
				}

			// Continue looping if (any) Packet has been detected
			} while (bytesReceived > 0);
		}

		// Number of data left to transmit should be zero!
		assert(currentSizeOfData == 0);

		// Update List of Lost Packets
		for (uint32_t i = 0; i < window.Size(); ++i)
		{
			if (sentPacketsAckBitfield[i] == false)
			{	// Packet was never acknowledged, save result to be handled later
				packetsLost.push_back(window[i]);
			}
		}

		// Check the list of lost packets
		for (auto& packetInfo : packetsLost)
		{
			printf("Packet (%d) Lost\n", packetInfo.SequenceNumber);
		}
	}

	// Return Results
	return totalBytesSent;
}

int RUDPStream::Receive2(char * OutBuffer, uint32_t sizeOfBuffer)
{
	int bytesReceived;
	uint32_t size = sizeOfBuffer + 100;		// size+100 extra bytes for the header space
	uint8_t * buffer = new uint8_t[size];

	bool isSuccess;

	// Receive First Packet
	RPacket packet;
	for (;;)
	{
		// Receive bytes
		bytesReceived = recvfrom(mSocket, reinterpret_cast<char*>(buffer), size, 0, NULL, NULL);
		if (bytesReceived > 0)
		{	// Packet Received!

			// Convert Bytes into RUDP Packet
			isSuccess = packet.Deserialize(buffer);

			// Check it data is not an RUDP Packet
			if (isSuccess)
			{	// RUDP Packet Received
				
				// For Debug Purposes only
				printf("message received [%d bytes]:\t(seq:%d ack:%d ack_bit:%d)\n", bytesReceived, packet.Sequence(), packet.Ack(), packet.AckBitfield().bitfield);
				
				// Return results (Copy only the data into the OutBuffer)
				auto data = packet.Buffer();
				memcpy_s(OutBuffer, sizeOfBuffer, data.data(), data.size());
				return static_cast<int>(data.size());
			}
			else
			{	// Not an RUDP Packet
				// For Debug Purposes only
				printf("Not an RUDP Packet: expected id<%d>, actual id<%d>\n", packet.RUDP_ID, packet.Id());
			}
		}
		else if(WSAGetLastError() != WSAEWOULDBLOCK)
		{
			printf("recvfrom failed with error: %d\n", WSAGetLastError());
			return SOCKET_ERROR;
		}
	}

}*/

bool RUDPStream::IsConnectionTimeOut(std::chrono::high_resolution_clock::time_point startTime, uint32_t maxTimeOutMS)
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count() <= maxTimeOutMS;
}

bool RUDPStream::ReceiveRPacket(RPacket& OutPacket, uint32_t maxTimeOutMS)
{
	int bytesReceived;
	bool isSuccess;
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	do
	{
		// Get Incoming Packets
		bytesReceived = recvfrom(mSocket, reinterpret_cast<char *>(mBuffer), sBufferSize, 0, NULL, NULL);

		// Check if Packet is Received
		if (bytesReceived > 0)
		{	// Packet has been received!

			// Convert bytes to RUDP packet information
			isSuccess = OutPacket.Deserialize(mBuffer);

			// Check if Packet is RUDP Packet
			if (!isSuccess)
			{	// Packet is not an RUDP Packet, ignore...
				printf("Not an RUDP Packet: expected id<%d>, actual id<%d>\n", OutPacket.RUDP_ID, OutPacket.Id());
			}
			else
			{	// RPacket successfully received. Return successful
				return true;
			}
		}
		else if (WSAGetLastError() != WSAEWOULDBLOCK)
		{	// Error Reported!
			printf("recvfrom produced error: %ld\n", WSAGetLastError());
		}

		// Sleep to give CPU some rest
		Sleep(1);
	} while (IsConnectionTimeOut(startTime, maxTimeOutMS));

	// Connection Timed-out
	return false;
}

bool RUDPStream::SendRPacket(const RPacket & packet)
{
	std::vector<uint8_t> packetData = std::move(packet.Serialize());
	int packetSize = static_cast<int>(packetData.size());

	// Send Single-Data Packet
	int bytesSent = sendto(mSocket, reinterpret_cast<char *>(packetData.data()), packetSize, 0, &mToAddress, SOCK_ADDR_SIZE);
	
	// Check if Bytes were sent!
	if (bytesSent < 0)
	{
		printf("sendto failed with error: %d\n", WSAGetLastError());
		printf("Sending: socket<%d> address<%d.%d.%d.%d.%d.%d.%d.%d.%d.%d>\n", static_cast<int>(mSocket)
			, (uint8_t)(mToAddress.sa_data[0]), (uint8_t)(mToAddress.sa_data[1]), (uint8_t)(mToAddress.sa_data[2]), (uint8_t)(mToAddress.sa_data[3]), (uint8_t)(mToAddress.sa_data[4])
			, (uint8_t)(mToAddress.sa_data[5]), (uint8_t)(mToAddress.sa_data[6]), (uint8_t)(mToAddress.sa_data[7]), (uint8_t)(mToAddress.sa_data[8]), (uint8_t)(mToAddress.sa_data[9]));

		return false;
	}
	else if (bytesSent != packetSize)
	{
		printf("Entire Packet failed to send. Is this normal?\n");
		return false;
	}
	else if (bytesSent == 0)
	{
		printf("Byte sent is 0. This shouldn't happen! Look into what this means!\n");
		return false;
	}

	// Return Success!
	return true;
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

	// Declare this as a new message
	++mMessageId;

	// Check if Fragmentation is Required
	if (sizeOfData <= sMaximumDataSize)
	{	// No Fragmentation Necessary, Simply send a single packet!

		// Create Single-Data Packet
		packet.Initialize(mSequenceNumber, mRemoteSequenceNumber, 0U, mMessageId, 1U, data, sizeOfData);
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


		// Initialize "Sliding Window" characteristics
		ackbitfield_t sentPacketsAckBitfield;								// Tracks the Received Packets from the Receiver and sets the AckBitfield accordingly
		ackbitfield_t recvdPacketsAckBitfield;								// Tracks the Sent Packets Acknowledged by the Receiver
		SlidingWindow window;												// Sliding Window used to track the state of the Sent Packets
		PacketFrame currentFrame;
		PacketFrame poppedFrame;
		//std::vector<PacketFrame> packetsLost;

		// Initialize Acknowledgement characteristics
		int bytesReceived;
		bool isSuccess;
		std::chrono::high_resolution_clock::time_point lastTimeReceivedPacket = std::chrono::high_resolution_clock::now();

		// Send with fragment data
		for (int fragment = 1; fragment <= numberOfFragments; ++fragment)
		{
			// ====================================================================================
			// ============================== Send Packet of Data =================================
			// ====================================================================================

			// Get the Minimum Between the current bytes left and the largest sized packet to send
			numOfBytesToSend = (currentSizeOfData < sizeOfPacketToSend) ? currentSizeOfData : sizeOfPacketToSend;

			// Create Packet Data (of appropriate size)
			packet.Initialize(mSequenceNumber, mRemoteSequenceNumber, recvdPacketsAckBitfield, mMessageId, numberOfFragments, currentDataLocation, numOfBytesToSend);
			isSuccess = SendRPacket(packet);
			assert(isSuccess);		// No Reason for this to fail...

			// Create Current Packet Frame
			currentFrame.Reassign(mSequenceNumber, currentDataLocation, numOfBytesToSend, false);

			// Update Sliding Window
			isSuccess = window.PushBack(currentFrame, &poppedFrame);

			// Check if Sliding Window pushed out a value
			if (!isSuccess)
			{	// Handle the packet that did not go through
				if(poppedFrame.IsAcknowledged == false)
				{	// Packet was never acknowledged

					// TODO: Handle packet loss better...
					//packetsLost.push_back(poppedFrame);
					mSequenceNumber += numberOfFragments - fragment;
					return -1;
				}
			}

			// Update Acknowledgement Bits (Shift bits same as Sliding Window)
			sentPacketsAckBitfield.bitfield <<= 1;
			assert(sentPacketsAckBitfield.bit00 == false);		// Bit-shifting should initialize bit00 to 0, which is false.

			// Update Sequence Number
			++mSequenceNumber;

			// Update bytes left to send
			currentDataLocation += numOfBytesToSend;
			currentSizeOfData -= numOfBytesToSend;
			totalBytesSent += bytesSent;

			// DEBUG: For Debug Purposes only
			printf("message sent [%d bytes]:\t(seq:%d ack:%d ack_bit:%d msg_size:%d)\n"
				, bytesSent, packet.Sequence(), packet.Ack(), packet.AckBitfield().bitfield, numOfBytesToSend);


			// ====================================================================================
			// ============================ Receive Acknowledgement ===============================
			// ====================================================================================

			// Check for any Acknowledgements Received
			for (;;)
			{
				// Determine conditions if the last packet is being sent
				if (fragment == numberOfFragments)
				{	// The last packet has been sent, handle receiving of packets accordingly
					assert(currentSizeOfData == 0);				// Number of data left to transmit should be zero!

					// Calculate Time left to Wait
					uint32_t TimeToWait = mMaxConnectionTimeOut;
					TimeToWait -= static_cast<uint32_t>((std::chrono::high_resolution_clock::now() - lastTimeReceivedPacket).count());

					// Wait on Incoming Packets
					isSuccess = ReceiveRPacket(packet, TimeToWait);
				}
				else
				{	// More packets to be sent, Check (but do not wait) on Incoming Packets
					isSuccess = ReceiveRPacket(packet, 0U);
				}

				if (isSuccess)
				{	// RUDP Packet Received!
					assert(packet.MessageId() == mMessageId);	// Should be consistent

					// Record time as most recent received packet!
					lastTimeReceivedPacket = std::chrono::high_resolution_clock::now();

					// =================== Handle Sequence Number =================

					// Check if the Packet received is more recent
					if (IsSeq2MoreRecent(mRemoteSequenceNumber, packet.Sequence()))
					{	// Update last ack received to this packet's sequence
						int32_t newRemoteSequenceOffset = DiffFromSeq1ToSeq2(packet.Sequence(), mRemoteSequenceNumber);
						assert(newRemoteSequenceOffset >= 0);	// Assert Seq2 is in fact more recent

						mRemoteSequenceNumber = packet.Sequence();						// Update to latest remote sequence number
						recvdPacketsAckBitfield.bitfield <<= newRemoteSequenceOffset;	// Shift to match latest remote sequence number
					}
					else
					{	// Set Received Packet's Ackbit
						int32_t ackbitToSetPlusOne = DiffFromSeq1ToSeq2(mRemoteSequenceNumber, packet.Sequence());
						assert(ackbitToSetPlusOne >= 0);	// Assert Seq1 is in fact more recent

						if (ackbitToSetPlusOne > 0 && ackbitToSetPlusOne < RPacket::NumberOfAcksPerPacket)
						{	// Bit to Set is within [0,31]
							recvdPacketsAckBitfield.Set(ackbitToSetPlusOne - 1, true);
						}
						else
						{	// DEBUG: For Debug Purposes Only
							printf("Ack Received, but not able to Set value...\n");
						}
					}

					// =============== Handle Ack Number and Ack-Bitfield ============
					/*
					This is the most important part of the sending process. This test must pass in order for the send to be considered successful.
					*/

					// TODO: Properly determine of all packets have been acknowledged

					// Set the sentPacket's Ack Bitfield according to the Ack and AckBitfield received
					int sentAckBitfieldOffset = DiffFromSeq1ToSeq2(mSequenceNumber, packet.Ack());
					if (sentAckBitfieldOffset > 0 && sentAckBitfieldOffset < RPacket::NumberOfAcksPerPacket)
					{	// Bits to Set is within [0,31]
						sentAckBitfieldOffset = sentAckBitfieldOffset - 1;	// Get Offset range in [0,31]
						sentPacketsAckBitfield.Set(sentAckBitfieldOffset, false);										// Set the received Ack Number
						sentPacketsAckBitfield.bitfield |= (packet.AckBitfield().bitfield << sentAckBitfieldOffset);	// Set All Acks in received AckBitfield
					}
				}
				else
				{	// RUDP Packet not received

					// Assert that there's no Connection Time-Outs
					if (IsConnectionTimeOut(lastTimeReceivedPacket, mMaxConnectionTimeOut))
					{
						// TODO: Handle Connection Time-outs better...
						printf("No acknowledgement received during send. Connection Timed-Out.");

						mSequenceNumber += numberOfFragments - fragment;
						return 0;
					}
					// Return to sending packets
					break;	
				}
			}
		}
	}

	// Return Results
	return totalBytesSent;
}

int RUDPStream::Receive(char * OutBuffer, uint32_t sizeOfBuffer)
{
	uint32_t size = sizeOfBuffer + 100;		// size+100 extra bytes for the header space

	bool isSuccess;
	RPacket packet;
	uint32_t messageID;
	uint32_t fragmentCount;

	// Initialize "Sliding Window" characteristics
	ackbitfield_t sentPacketsAckBitfield;								// Tracks the Received Packets from the Receiver and sets the AckBitfield accordingly
	ackbitfield_t recvdPacketsAckBitfield;								// Tracks the Sent Packets Acknowledged by the Receiver
	PacketFrame currentFrame;
	PacketFrame poppedFrame;
	//std::vector<PacketFrame> packetsLost;

	uint8_t * copyPointer;
	std::vector<PacketFrame> sequencedData;
	SlidingWindow window;
	ackbitfield_t packetAcknowledged;
	uint32_t oldestRSNfromWindow = mRemoteSequenceNumber;
	uint32_t newestRSNfromWindow = mRemoteSequenceNumber;

	//uint32_t tempRemoteSequenceNumber = mRemoteSequenceNumber;
	uint32_t receivedRemoteSequenceNumber;

	bool isFirstPacket = true;
	int32_t packetOffset;
	for (;;)
	{
		// ====================================================================================
		// ================================== Receive RPacket =================================
		// ====================================================================================

		isSuccess = ReceiveRPacket(packet, mMaxConnectionTimeOut);

		// Connection Timed-Out
		if (!isSuccess)
		{
			if (!isFirstPacket)
			{	// Adjust expected sequence number
				mRemoteSequenceNumber += fragmentCount;
			}

			printf("Connection Timed-Out");
			return 0;
		}

		// Calculate how many packets ahead this packet is
		receivedRemoteSequenceNumber = packet.Sequence();
		packetOffset = DiffFromSeq1ToSeq2(oldestRSNfromWindow, receivedRemoteSequenceNumber);
		assert(oldestRSNfromWindow + packetOffset == receivedRemoteSequenceNumber);			// Assert the DiffFromFunction works

		// Determine if Packet is within a valid range
		if (packetOffset < 0 || packetOffset >= RPacket::NumberOfAcksPerPacket)
		{
			printf("RUDP Packet is not within a valid range: current<%d>, received<%d>, dif<%d>\n"
				, oldestRSNfromWindow, packet.Sequence(), packetOffset);

			continue;
		}

		assert(IsSeq2MoreRecent(oldestRSNfromWindow, receivedRemoteSequenceNumber) == true);	// Assert these functions work!

		// ========== Handle First Packet ===========

		// Handle Case of first packet of message received
		if (isFirstPacket)
		{	
			isFirstPacket = false;

			assert(mMessageId + 1 == packet.MessageId());
			mMessageId = packet.MessageId();
			fragmentCount = packet.FragmentCount();

			assert(packetOffset <= fragmentCount);	// Assert that the packet received is not more than the fragment count
		}
		else
		{	// Do some asserts
			assert(mMessageId == packet.MessageId());
		}

		// ============================ Handling Sequence Numbers =============================

		packetOffset = DiffFromSeq1ToSeq2(newestRSNfromWindow, receivedRemoteSequenceNumber);

		// Determine what to do with packet
		if (packetOffset > 0)
		{	// Packet is newer, adjust window accordingly
			assert(IsSeq2MoreRecent(newestRSNfromWindow, receivedRemoteSequenceNumber) == true);	// Assert IsSeq2MoreRecent functions work!

			// Shift Window accordingly
			uint32_t i;
			for (i = 0; i < static_cast<uint32_t>(packetOffset); ++i)
			{
				// Update Acknowledgement Bits (Shift bits same as Sliding Window)
				recvdPacketsAckBitfield.bitfield <<= 1;
				recvdPacketsAckBitfield.bit00 = window.Top().IsAcknowledged;

				// Assign Bad Packet Frame
				currentFrame.Reassign(newestRSNfromWindow + i);

				// Update Sliding Window
				isSuccess = window.PushBack(currentFrame, &poppedFrame);

				// Check if Sliding Window pushed out a value
				if (!isSuccess)
				{	// Handle the packet that did not go through
					if (poppedFrame.IsAcknowledged == false)
					{	// Packet was never acknowledged, return unsuccessful submission

						// TODO: Handle the case if packets are not to be re-sent...
						printf("Packet Loss Detected! Receive Cannot complete. . .");
						
						// Adjust expected sequence number
						mRemoteSequenceNumber += fragmentCount;
						return -1;
					}
					else
					{	// Packet Acknowledge, push out as sequenced data
						sequencedData.push_back(currentFrame);
					}
				}
			}

			// Save Buffer in Sequence
			currentFrame.Reassign(receivedRemoteSequenceNumber, packet.Buffer(), true);
		}
		else if(packetOffset < 0)
		{	// Packet is older, update window accordingly
			assert(IsSeq2MoreRecent(newestRSNfromWindow, receivedRemoteSequenceNumber) == false);	// Assert IsSeq2MoreRecent functions work!
			assert(packetOffset >= (1 - RPacket::NumberOfAcksPerPacket));															// Assert OldestRNS clamped packets properly
			
			int32_t index = (RPacket::NumberOfAcksPerPacket - 1) + packetOffset;

			// Check if packet is acknowledged
			if (!packetAcknowledged[index])
			{	// Packet not acknowledged, update Packet Frame state
				packetAcknowledged.Set(index, true);
				PacketFrame& frame = window[index];

				// Transfer memory over to sequenced data
				auto& buffer = packet.Buffer();
				auto bufferSize = buffer.size();
				copyPointer = reinterpret_cast<uint8_t *>(malloc(bufferSize));
				memcpy(copyPointer, buffer.data(), static_cast<size_t>(bufferSize));

				// Save Buffer in Sequence
				frame.Reassign(receivedRemoteSequenceNumber, packet.Buffer(), true);
			}
			else
			{	// Packet already acknowledged, redundant packet ignored...
				printf("Redundant Packet Detected: seq<%d>", receivedRemoteSequenceNumber);
				continue;
			}
		}
		else
		{	// Packet is currently newest packet received prior, redunant packet ignored...
			printf("Redundant Packet Detected: seq<%d>", receivedRemoteSequenceNumber);
			continue;
		}




		// ====================================================================================
		// =========================== Send / Return Acknowledgement ==========================
		// ====================================================================================

		// Send Acknowledgement Information
		packet.Initialize(mSequenceNumber + 1, newestRSNfromWindow, packetAcknowledged, mMessageId, 1, std::vector<uint8_t>());
		isSuccess = SendRPacket(packet);

		assert(isSuccess);		// No reason for this to fail...

		// TODO: Add End State to this infinite loop!!!!
	}

	// TODO: Sequence Data that is stored in SequencedData

	// Compute Total Bytes Received
	uint32_t totalBytesReceived = 0;
	for (auto& frame : sequencedData)
	{
		totalBytesReceived += frame.SizeOfData;
	}

	if (totalBytesReceived > sizeOfBuffer)
	{	// TODO: Handle this case better!
		return -1;
	}
	
	// Copy Sequenced Data into Buffer
	char * CurrentBufferLocation = OutBuffer;
	for (auto& frame : sequencedData)
	{
		memcpy(CurrentBufferLocation, frame.DataPointer, frame.SizeOfData);
		CurrentBufferLocation += frame.SizeOfData;
	}

	// Return Successful Receipt of Data
	return totalBytesReceived;
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
