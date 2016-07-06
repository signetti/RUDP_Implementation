#include "stdafx.h"
#include "RUDPStream.h"
#include "RPacket.h"
#include <assert.h>
#include <time.h>

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

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	localtime_s(&tstruct, &now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
}

bool RUDPStream::IsConnectionTimeOut(std::chrono::high_resolution_clock::time_point startTime, int32_t maxTimeOutMS, uint64_t* OutDuration)
{
	if (OutDuration != nullptr)
	{
		uint64_t& duration = *(OutDuration);
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
		return duration > maxTimeOutMS;
	}
	else
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count() > maxTimeOutMS;
	}
}

bool RUDPStream::ReceiveRPacket(uint32_t & OutBytesReceived, RPacket & OutPacket, int32_t maxTimeOutMS)
{
	int bytesReceived;
	bool isSuccess;
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	do
	{
		// Get Incoming Packets
		bytesReceived = recvfrom(mSocket, reinterpret_cast<char *>(mBuffer), sBufferSize, 0, NULL, NULL);

		// Record Received Bytes
		OutBytesReceived = (bytesReceived > 0) ? static_cast<uint32_t>(bytesReceived) : 0U;

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

				// DEBUG: For Debugging Purposes only
				printf("---- Packet Rcvd [%s]: seq<%d> ack<%d> ack_bit<%X> msg_id<%d> frag<%d> num_bytes_rcvd<%d>\n"
					, currentDateTime().c_str(), OutPacket.Sequence(), OutPacket.Ack(), OutPacket.AckBitfield().bitfield
					, OutPacket.MessageId(), OutPacket.FragmentCount(), static_cast<uint32_t>(OutPacket.Buffer().size()));

				return true;
			}
		}
		else if (WSAGetLastError() != WSAEWOULDBLOCK)
		{	// Error Reported!
			printf("recvfrom produced error: %ld\n", WSAGetLastError());
		}

		// Sleep to give CPU some rest
		Sleep(1);
	} while (!IsConnectionTimeOut(startTime, maxTimeOutMS));

	// Connection Timed-out
	return false;
}

bool RUDPStream::ReceiveRPacket(RPacket& OutPacket, int32_t maxTimeOutMS)
{
	uint32_t farse;
	return ReceiveRPacket(farse, OutPacket, maxTimeOutMS);
}

bool RUDPStream::SendRPacket(uint32_t& OutBytesSent, const RPacket & packet)
{
	std::vector<uint8_t> packetData = std::move(packet.Serialize());
	int packetSize = static_cast<int>(packetData.size());

	// Send Single-Data Packet
	int bytesSent = sendto(mSocket, reinterpret_cast<char *>(packetData.data()), packetSize, 0, &mToAddress, SOCK_ADDR_SIZE);

	// Record Sent Bytes
	OutBytesSent = (bytesSent > 0) ? static_cast<uint32_t>(bytesSent) : 0U;

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


	// DEBUG: For Debugging Purposes only
	printf("---- Packet Sent [%s]: seq<%d> ack<%d> ack_bit<%X> msg_id<%d> frag<%d> num_bytes_sent<%d>\n"
		, currentDateTime().c_str(), packet.Sequence(), packet.Ack(), packet.AckBitfield().bitfield
		, packet.MessageId(), packet.FragmentCount(), static_cast<uint32_t>(packet.Buffer().size()));

	// Return Success!
	return true;
}

bool RUDPStream::SendRPacket(const RPacket & packet)
{
	uint32_t farse;
	return SendRPacket(farse, packet);
}

int RUDPStream::Send(const uint8_t * data, uint32_t sizeOfData)
{

	// Handle Special Cases
	if (sizeOfData == 0 || data == nullptr)
	{
		return -1;
	}

	uint32_t totalBytesSent = 0;
	RPacket packet;
	bool isSuccess;

	// ======= Send the Data in Packets =====

	// Convert Size to Integer Data
	int32_t currentSizeOfData = static_cast<int32_t>(sizeOfData);
	if (currentSizeOfData < 0)
	{	// Size overflowed, size is too large, don't even send
		return -1;
	}

	// Declare this as a new message
	++mMessageId;

	// DEBUG: For Debugging Purposes only
	printf("==== Begin Sending [%s]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d>\n"
		, currentDateTime().c_str(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut);

	// Check if Fragmentation is Required
	if (sizeOfData <= sMaximumDataSize)
	{	// No Fragmentation Necessary, Simply send a single packet!

		// Create Single-Data Packet
		packet.Initialize(mSequenceNumber, mRemoteSequenceNumber, 0U, mMessageId, 1U, data, sizeOfData);
		std::vector<uint8_t> packetData = packet.Serialize();

		// Send Single-Data Packet
		totalBytesSent = sendto(mSocket, reinterpret_cast<char *>(packetData.data()), static_cast<int>(packetData.size()), 0, &mToAddress, SOCK_ADDR_SIZE);
		isSuccess = SendRPacket(totalBytesSent, packet);
		assert(isSuccess);		// No Reason for this to fail...

		// Update Sequence Number
		mSequenceNumber++;

		// DEBUG: For Debug Purposes only
		//printf("message sent [%d bytes]:\t(seq:%d ack:%d ack_bit:%d msg_size:%d)\n"
		//	, totalBytesSent, packet.Sequence(), packet.Ack(), packet.AckBitfield().bitfield, sizeOfData);
	}
	else
	{	// Fragmentation Required!

		// Calculate number and size of Fragments (using Ceiling Function)
		int numberOfFragments = ((sizeOfData - 1) / sMaximumDataSize) + 1;
		int sizeOfPacketToSend = ((sizeOfData - 1) / numberOfFragments) + 1;

		const uint8_t * currentDataLocation = data;
		int numOfBytesToSend;
		uint32_t bytesSent;


		// Initialize "Sliding Window" characteristics
		ackbitfield_t sentPacketsAckBitfield(true);							// Tracks the Received Packets from the Receiver and sets the AckBitfield accordingly
		bool isMostRecentAcknowledged;
		ackbitfield_t recvdPacketsAckBitfield(true);						// Tracks the Sent Packets Acknowledged by the Receiver
		SlidingWindow window;												// Sliding Window used to track the state of the Sent Packets
		PacketFrame currentFrame;
		PacketFrame poppedFrame;
		//std::vector<PacketFrame> packetsLost;

		// Initialize Acknowledgement characteristics
		std::chrono::high_resolution_clock::time_point lastTimeReceivedPacket = std::chrono::high_resolution_clock::now();

		uint32_t lastReceivedRemoteSequenceNumber = mRemoteSequenceNumber - 1;

		// Send with fragment data
		for (int fragment = 1; fragment <= numberOfFragments; ++fragment)
		{
			// ====================================================================================
			// ============================== Send Packet of Data =================================
			// ====================================================================================

			// Get the Minimum Between the current bytes left and the largest sized packet to send
			numOfBytesToSend = (currentSizeOfData < sizeOfPacketToSend) ? currentSizeOfData : sizeOfPacketToSend;

			// Create Packet Data (of appropriate size)
			packet.Initialize(mSequenceNumber, lastReceivedRemoteSequenceNumber + 1, recvdPacketsAckBitfield, mMessageId, numberOfFragments, currentDataLocation, numOfBytesToSend);
			isSuccess = SendRPacket(bytesSent, packet);
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
					mRemoteSequenceNumber += numberOfFragments;
					return -1;
				}
			}

			// Update Acknowledgement Bits (Shift bits same as Sliding Window)
			sentPacketsAckBitfield.bitfield <<= 1;
			assert(sentPacketsAckBitfield.bit00 == false);		// Bit-shifting should initialize bit00 to 0, which is false.

			// Update Sequence Number
			++mSequenceNumber;
			isMostRecentAcknowledged = false;

			// Update bytes left to send
			currentDataLocation += numOfBytesToSend;
			currentSizeOfData -= numOfBytesToSend;
			totalBytesSent += bytesSent;

			// DEBUG: For Debug Purposes only
			//printf("message sent [%d bytes]:\t(seq:%d ack:%d ack_bit:%d msg_size:%d)\n"
			//	, bytesSent, packet.Sequence(), packet.Ack(), packet.AckBitfield().bitfield, numOfBytesToSend);


			// ====================================================================================
			// ============================ Receive Acknowledgement ===============================
			// ====================================================================================

			// Check for any Acknowledgements Received
			for (;;)
			{
				// Determine conditions if the last packet is being sent
				if (fragment >= numberOfFragments - 1)
				{	// The last packet has been sent, handle receiving of packets accordingly
					// Calculate Time left to Wait
					uint32_t timeSinceLastReceived = static_cast<uint32_t>((std::chrono::high_resolution_clock::now() - lastTimeReceivedPacket).count());

					// Wait on Incoming Packets
					isSuccess = ReceiveRPacket(packet, mMaxConnectionTimeOut - timeSinceLastReceived);
				}
				else
				{	// More packets to be sent, Check (but do not wait) on Incoming Packets
					isSuccess = ReceiveRPacket(packet, 0U);
				}

				if (isSuccess)
				{	// RUDP Packet Received!
					assert(packet.MessageId() == mMessageId);	// Should be consistent

					if (packet.Sequence() < mRemoteSequenceNumber)
					{	// Old Packet received...
						printf("Received packet for new message detected, ignored...\n");
						continue;
					}
					else if (packet.Sequence() >= mRemoteSequenceNumber + numberOfFragments)
					{	// Packet outside of range received...
						printf("Received packet for new message detected, ignored...\n");
						continue;
					}

					// Record time as most recent received packet!
					lastTimeReceivedPacket = std::chrono::high_resolution_clock::now();


					// =================== Handle Sequence Number =================

					int32_t packetOffset = DiffFromSeq1ToSeq2(lastReceivedRemoteSequenceNumber, packet.Sequence());

					// Check if the Packet received is more recent
					if (packetOffset > 0)
					{	// Update last ack received to this packet's sequence

						// Create Bitmask to check all the ack_bits that will be shifted off (starting from the end bit31)
						uint32_t bit_mask = ~(0U);	// All start as true (1)
						bit_mask >>= packetOffset;	// Make the last N bits equal false (0)... this will be our mask to check if the last N bits are true

						if (~(recvdPacketsAckBitfield.bitfield | bit_mask) != 0U)
						{	// If the last N bits do not all equal true (N = packetOffset), then at least one packet shifted was not acknowledged

							// TODO: Handle unacknowledged packet...
							printf("Packet Loss detected, but currently ignoring...\n");
						}

						// Include all 32 results of acknowledged bitfield received (using OR gate logic)
						recvdPacketsAckBitfield.bitfield <<= packetOffset;
						recvdPacketsAckBitfield.Set(packetOffset - 1, true);	// The bit for the last Received Remote Sequence Number needs to get set!

						lastReceivedRemoteSequenceNumber = packet.Sequence();		// Update to latest remote sequence number
					}
					else if(packetOffset < 0)
					{	// Set Received Packet's Ackbit

						packetOffset = -packetOffset;

						if (packetOffset >= RPacket::NumberOfAcksPerPacket)
						{	// DEBUG: For Debug Purposes Only
							printf("Ack Received, but not able to Set value...\n");
						}
						else
						{	// Set the ack_bit for the respective acknowledged number received
							recvdPacketsAckBitfield.Set(packetOffset - 1, true);
							
							// Include all 32 results of acknowledged bitfield received (using OR gate logic)
							recvdPacketsAckBitfield.bitfield |= (packet.AckBitfield().bitfield << packetOffset);
						}
					}

					// =============== Handle Ack Number and Ack-Bitfield ============
					// This is the most important part of the sending process.
					// This test must pass in order for the send to be considered successful.


					// Set the Sent Packet's Ack Bitfield according to the Ack and AckBitfield received
					// (recall Seq Number is 1 ahead of what's sent, and Ack Numbers await for the Seq Number ahead)
					packetOffset = DiffFromSeq1ToSeq2(packet.Ack(), mSequenceNumber);
					assert(packetOffset >= 0);		// Cannot acknowledge a sequence that is not sent
					
					if (packetOffset >= 0)
					{
						// Check most recent
						if (packetOffset == 0)
						{	// This is most recent
							isMostRecentAcknowledged = true;
						}
						else
						{	// Set respective bit in bit-field
							sentPacketsAckBitfield.Set(packetOffset - 1, true);
						}

						// Set the received Ack Number
						sentPacketsAckBitfield.bitfield |= (packet.AckBitfield().bitfield << packetOffset);	// Set All Acks in received AckBitfield
					}
				}
				else if(fragment >= numberOfFragments - 1)
				{	// RUDP Packet not received

					// Assert that there's no Connection Time-Outs
					uint64_t duration;
					if (IsConnectionTimeOut(lastTimeReceivedPacket, mMaxConnectionTimeOut, &duration))
					{
						// TODO: Handle Connection Time-outs better...
						printf("No acknowledgement received during send. Connection Timed-Out: max_time<%d> duration<%lld>\n", mMaxConnectionTimeOut, duration);

						mSequenceNumber += numberOfFragments - fragment;
						mRemoteSequenceNumber += numberOfFragments;
						return -1;
					}
				}


				// ============== Check Condition to Continue Receiving Packets =================
				if (fragment < numberOfFragments - 1)
				{	// Still fragments left to send, keep sending
					break;
				}
				else if (isMostRecentAcknowledged && sentPacketsAckBitfield.IsAll(true))
				{	// The Second-to-Last fragment: With all acknowledgements received send last fragment packet with all acknowledgements
					// The Last fragment: With all acknowledgements received, exit the loop successful!

					mRemoteSequenceNumber = lastReceivedRemoteSequenceNumber + 1;
					break;
				}
			}
		}
	}

	// DEBUG: For Debug Purposes Only!
	printf("==== Successful Sending [%s]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> msg_size<%d> bytes_sent<%d>\n"
		, currentDateTime().c_str(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut, sizeOfData, totalBytesSent);

	assert(currentSizeOfData == 0);				// Number of data left to transmit should be zero!

	// Return Results
	return sizeOfData;
}

int RUDPStream::Receive(char * OutBuffer, uint32_t sizeOfBuffer)
{
	bool isSuccess;
	RPacket packet;
	uint32_t fragmentCount = 1;

	// Initialize "Sliding Window" characteristics
	uint32_t lastSentPacketAcknowledged = mSequenceNumber - 1;
	ackbitfield_t sentPacketsAckBitfield(true);								// Tracks the Sent Packets Acknowledged by the Receiver
	ackbitfield_t recvdPacketsAckBitfield(true);							// Tracks the Received Packets from the Receiver and sets the AckBitfield accordingly
	
	PacketFrame currentFrame;
	PacketFrame poppedFrame;

	std::vector<PacketFrame> sequencedData;
	SlidingWindow window;
	uint32_t oldestRSNfromWindow = mRemoteSequenceNumber - 1;
	uint32_t newestRSNfromWindow = mRemoteSequenceNumber - 1;

	//uint32_t tempRemoteSequenceNumber = mRemoteSequenceNumber;
	uint32_t receivedRemoteSequenceNumber;
	uint32_t currentSequenceNumber = mSequenceNumber;

	bool isFirstPacket = true;
	int32_t packetOffset;

	// Declare this as a new message
	++mMessageId;

	// DEBUG: For Debugging Purposes only
	printf("==== Begin Receiving [%s]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d>\n"
		, currentDateTime().c_str(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut);

	std::chrono::high_resolution_clock::time_point timeSinceLastValidPacketReceived = std::chrono::high_resolution_clock::now();
	uint32_t timeSinceLastValidPacket;
	for (;;)
	{
		// ====================================================================================
		// ================================== Receive RPacket =================================
		// ====================================================================================

		timeSinceLastValidPacket = static_cast<uint32_t>((std::chrono::high_resolution_clock::now() - timeSinceLastValidPacketReceived).count());
		isSuccess = ReceiveRPacket(packet, mMaxConnectionTimeOut - timeSinceLastValidPacket);

		// Connection Timed-Out
		if (!isSuccess)
		{
			// TODO: Handle Connection time-outs better
			if (!isFirstPacket)
			{	// Adjust expected sequence number
				mSequenceNumber += fragmentCount;
				mRemoteSequenceNumber += fragmentCount;
			}

			printf("Connection Timed-Out\n");
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
				, oldestRSNfromWindow, receivedRemoteSequenceNumber, packetOffset);

			continue;
		}

		if (receivedRemoteSequenceNumber < mRemoteSequenceNumber)
		{	// Old Packet received...
			printf("Packet for new message detected, but ignored...\n");
			continue;
		}
		else if (!isFirstPacket && receivedRemoteSequenceNumber >= mRemoteSequenceNumber + fragmentCount)
		{	// Packet outside of range received...
			printf("Packet for new message detected, but ignored...\n");
			continue;
		}
		

		// Assert that the message is correct
		assert(isFirstPacket || mMessageId == packet.MessageId());


		// Assert these functions work!
		//assert(IsSeq2MoreRecent(oldestRSNfromWindow, receivedRemoteSequenceNumber) == true || oldestRSNfromWindow == receivedRemoteSequenceNumber);

		// Packet is deemed valid, continue...
		timeSinceLastValidPacketReceived = std::chrono::high_resolution_clock::now();

		// ============================== Handle First Packet =================================

		// Handle Case of first packet of message received
		if (isFirstPacket)
		{	
			fragmentCount = packet.FragmentCount();

			isFirstPacket = false;
			assert(static_cast<uint32_t>(packetOffset) <= fragmentCount);	// Assert that the packet received is not more than the fragment count
		}
		else
		{	// Do some asserts
			assert(fragmentCount == packet.FragmentCount());
		}

		// ============================ Handling Sequence Numbers =============================

		packetOffset = DiffFromSeq1ToSeq2(newestRSNfromWindow, receivedRemoteSequenceNumber);

		// Determine what to do with packet
		if (packetOffset > 0)
		{	// Packet is newer, adjust window accordingly

			// Shift Window accordingly
			int32_t i;
			for (i = 1; i <= packetOffset; ++i)
			{
				// Create Packet Frame
				if (i != packetOffset)
				{	// Assign Bad Packet Frame
					currentFrame.Reassign(newestRSNfromWindow + i);
				}
				else
				{	// Assign Current Received Packet into Packet Frame
					assert(newestRSNfromWindow + i == receivedRemoteSequenceNumber);
					currentFrame.Reassign(receivedRemoteSequenceNumber, packet.Buffer(), true);
				}

				// Update Acknowledgement Bits (Shift bits same as Sliding Window)
				recvdPacketsAckBitfield.bitfield <<= 1;
				recvdPacketsAckBitfield.bit00 = window.IsEmpty() || window.Top().IsAcknowledged;

				// Update Sliding Window
				isSuccess = window.PushBack(currentFrame, &poppedFrame);

				// Check if Sliding Window pushed out a value
				if (!isSuccess)
				{	// Handle the packet that did not go through
					if (poppedFrame.IsAcknowledged == false)
					{	// Packet was never acknowledged, return unsuccessful submission

						// TODO: Handle packet loss case better...
						printf("Packet Loss Detected! Receive Cannot complete. . .\n");
						
						// Adjust expected sequence number
						mSequenceNumber += fragmentCount;
						mRemoteSequenceNumber += fragmentCount;
						return -1;
					}
					else
					{	// Packet Acknowledge, push out as sequenced data
						sequencedData.push_back(currentFrame);
					}
				}
			}

			// Update newest remote sequence number
			newestRSNfromWindow = receivedRemoteSequenceNumber;
		}
		else if(packetOffset < 0)
		{	// Packet is older, update window accordingly
			
			assert(packetOffset >= (0 - RPacket::NumberOfAcksPerPacket));															// Assert OldestRNS clamped packets properly
			
			int32_t index = (-1) - packetOffset;//(RPacket::NumberOfAcksPerPacket - 1) + packetOffset;

			// Check if packet is acknowledged
			if (!recvdPacketsAckBitfield[index])
			{	// Packet not acknowledged, update Packet Frame state
				recvdPacketsAckBitfield.Set(index, true);
				PacketFrame& frame = window[index + 1];

				// Save Buffer in Sequence
				frame.Reassign(receivedRemoteSequenceNumber, packet.Buffer(), true);
			}
			else
			{	// Packet already acknowledged, redundant packet ignored...
				printf("Redundant Packet Detected: seq<%d>\n", receivedRemoteSequenceNumber);
				continue;
			}
		}
		else
		{	// Packet is currently newest packet received prior, redunant packet ignored...
			assert(!window.IsEmpty() || window.Top().IsAcknowledged);

			printf("Redundant Packet Detected: seq<%d>\n", receivedRemoteSequenceNumber);
			continue;
		}

		// ============================ Handling Acknowledgements =============================
		// DEBUG:
		if (newestRSNfromWindow == mRemoteSequenceNumber + fragmentCount - 1)
		{
			fragmentCount = fragmentCount;
		}

		if (isFirstPacket)
		{
			assert(sentPacketsAckBitfield.bitfield == ~(0U));	// The sent ack_bitfield must all be initialized to true to avoid confusion...
			assert(DiffFromSeq1ToSeq2(packet.Ack(), mSequenceNumber) == 0);	// Assert that the two connections are synced...
			
			lastSentPacketAcknowledged = mSequenceNumber - 1;
		}
		else
		{
			packetOffset = DiffFromSeq1ToSeq2(lastSentPacketAcknowledged, packet.Ack());

			if (packetOffset > 0)
			{	// More recent acknowledged sequence number received

				// Create Bitmask to check all the ack_bits that will be shifted off (starting from the end bit31)
				uint32_t bit_mask = ~(0U);	// All start as true (1)
				bit_mask >>= packetOffset;	// Make the last N bits equal false (0)... this will be our mask to check if the last N bits are true
				
				if (~(sentPacketsAckBitfield.bitfield | bit_mask) != 0U)
				{	// If the last N bits do not all equal true (N = packetOffset), then at least one packet shifted was not acknowledged

					// TODO: Handle unacknowledged packet...
					printf("Packet Loss detected, but currently ignoring...\n");
				}

				// Include all 32 results of acknowledged bitfield received (using OR gate logic)
				sentPacketsAckBitfield.bitfield <<= packetOffset;
				sentPacketsAckBitfield.bitfield |= packet.AckBitfield().bitfield;

				// Update last packet acknowledged to this ack number
				lastSentPacketAcknowledged = packet.Ack();
			}
			else if (packetOffset < 0)
			{
				packetOffset = -packetOffset;

				if (packetOffset >= RPacket::NumberOfAcksPerPacket)
				{	// DEBUG: For Debug Purposes Only
					printf("Ack Received, but not able to Set value...\n");
				}
				else
				{	// Set the ack_bit for the respective acknowledged number received
					sentPacketsAckBitfield.Set(packetOffset - 1, true);

					// Include all 32 results of acknowledged bitfield received (using OR gate logic)
					sentPacketsAckBitfield.bitfield |= (packet.AckBitfield().bitfield << packetOffset);
				}
			}
			else
			{
				// Include all 32 results of acknowledged bitfield received (using OR gate logic) in case of update...
				sentPacketsAckBitfield.bitfield |= packet.AckBitfield().bitfield;
				printf("Redundant acknowledgement ignored: ack<%d>\n", packet.Ack());
			}
		}


		// ====================================================================================
		// =========================== Send / Return Acknowledgement ==========================
		// ====================================================================================

		// Send Acknowledgement Information
		packet.Initialize(currentSequenceNumber, newestRSNfromWindow + 1, recvdPacketsAckBitfield, mMessageId, 1, std::vector<uint8_t>());
		isSuccess = SendRPacket(packet);
		assert(isSuccess);		// No reason for this to fail...

		// Update Sequence Number
		++currentSequenceNumber;

		// TODO: Add End State to this infinite loop!!!!

		// Check if all acks received...


		if ((newestRSNfromWindow == mRemoteSequenceNumber + fragmentCount - 1) && (recvdPacketsAckBitfield.IsAll(true)))
		{
			if ((lastSentPacketAcknowledged == currentSequenceNumber - 1) && (sentPacketsAckBitfield.IsAll(true)))
			{	// True perfection has been reached in transmitting this message. Break free!
				// Adjust expected sequence number
				mSequenceNumber += fragmentCount;
				mRemoteSequenceNumber += fragmentCount;
				break;
			}
		}
	}

	// Remove all frames from Window
	while (!window.IsEmpty())
	{
		sequencedData.push_back(window.PopBack());
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
		printf("Bytes received waaay too large: max<%d> rcvd<%d>\n", sizeOfBuffer, totalBytesReceived);
		return -1;
	}
	else if (totalBytesReceived <= 0)
	{
		printf("Bytes received is bad: rcvd<%d>\n", totalBytesReceived);
	}
	else
	{	// DEBUG: For Debugging Purposes only
		printf("==== Successful Receiving [%s]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> bytes_rcvd<%d>\n"
			, currentDateTime().c_str(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut, totalBytesReceived);
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
