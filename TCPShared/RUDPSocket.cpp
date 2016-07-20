#include "stdafx.h"
#include "RUDPSocket.h"

#include "RPacket.h"

#include "Logger.h"
#include <chrono>	// For CurrentTime()
#include <assert.h>	// For Testing

LOGGER_SET_FILE_STATE(false, true, BasicColor::WHITE);

// ============================= Helper Functions ================================

const uint64_t CurrentTime(bool reset = false)
{
	// DEBUG: Record time in between
	static std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	if (reset)
	{
		startTime = std::chrono::high_resolution_clock::now();
		return 0;
	}
	else
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
	}
}

static int32_t DiffFromSeq1ToSeq2(seq_num_t seq1, seq_num_t seq2)
{
	constexpr static const seq_num_t MaximumSequenceNumberValue = ~(seq_num_t(0U));

	uint32_t diff;
	if (seq1 > seq2)
	{	// Difference will result in negative value
		diff = seq1 - seq2;
		if (diff > (MaximumSequenceNumberValue / 2))
		{	// Negative value is actually Positive (due to wrap around)
			return static_cast<int32_t>((MaximumSequenceNumberValue / 2) - diff);
		}
		else
		{	// Return negative difference
			return 0 - static_cast<int32_t>(diff);
		}
	}
	else
	{	// Difference will result in positive value
		diff = seq2 - seq1;
		if (diff > (MaximumSequenceNumberValue / 2))
		{	// Positive value is actually Negative (due to wrap around)
			return 0 - static_cast<int32_t>((MaximumSequenceNumberValue / 2) - diff);
		}
		else
		{	// Return positive difference
			return static_cast<int32_t>(diff);
		}
	}
}

// =================================== Packet Frame =====================================

RUDPSocket::PacketFrame::PacketFrame() : SequenceNumber(0), DataPointer(nullptr), SizeOfData(0), IsAcknowledged(false), IsDataCloned(false) {}

RUDPSocket::PacketFrame::PacketFrame(const PacketFrame& other)\
	: SequenceNumber(other.SequenceNumber), IsAcknowledged(other.IsAcknowledged)
	, SizeOfData(other.SizeOfData), IsDataCloned(other.IsDataCloned)
{
	if (IsDataCloned)
	{
		// Transfer memory over to DataPointer
		uint8_t * clone = reinterpret_cast<uint8_t *>(malloc(SizeOfData));
		memcpy_s(clone, static_cast<size_t>(SizeOfData), other.DataPointer, static_cast<size_t>(SizeOfData));

		DataPointer = clone;
	}
	else
	{
		DataPointer = other.DataPointer;
	}
}

RUDPSocket::PacketFrame& RUDPSocket::PacketFrame::operator=(const PacketFrame& rhs)
{
	if (this != &rhs)
	{
		SequenceNumber = rhs.SequenceNumber;
		IsAcknowledged = rhs.IsAcknowledged;
		SizeOfData = rhs.SizeOfData;
		IsDataCloned = rhs.IsDataCloned;

		if (IsDataCloned)
		{
			// Transfer memory over to DataPointer
			uint8_t * clone = reinterpret_cast<uint8_t *>(malloc(SizeOfData));
			memcpy_s(clone, static_cast<size_t>(SizeOfData), rhs.DataPointer, static_cast<size_t>(SizeOfData));

			DataPointer = clone;
		}
		else
		{
			DataPointer = rhs.DataPointer;
		}
	}
	return *this;
}

RUDPSocket::PacketFrame::PacketFrame(PacketFrame&& other) : SequenceNumber(other.SequenceNumber), IsAcknowledged(other.IsAcknowledged)
, DataPointer(other.DataPointer), SizeOfData(other.SizeOfData), IsDataCloned(other.IsDataCloned)
{
	other.DataPointer = nullptr;
	other.SizeOfData = 0;
	other.IsDataCloned = false;
}

RUDPSocket::PacketFrame& RUDPSocket::PacketFrame::operator=(PacketFrame&& rhs)
{
	if (this != &rhs)
	{
		SequenceNumber = rhs.SequenceNumber;
		IsAcknowledged = rhs.IsAcknowledged;
		DataPointer = rhs.DataPointer;
		SizeOfData = rhs.SizeOfData;
		IsDataCloned = rhs.IsDataCloned;

		rhs.DataPointer = nullptr;
		rhs.SizeOfData = 0;
		rhs.IsDataCloned = false;
	}
	return *this;
}

RUDPSocket::PacketFrame::~PacketFrame()
{	// Free memory
	if (IsDataCloned && DataPointer != nullptr)
	{
		//free(DataPointer);
	}
}

// Assign Packet Frame as Bad
void RUDPSocket::PacketFrame::Reassign(uint32_t newSequenceNumber)
{
	Reassign(newSequenceNumber, nullptr, 0, false);
}

// Assign Packet Frame, with the data signature
void RUDPSocket::PacketFrame::Reassign(uint32_t newSequenceNumber, const uint8_t * newDataPointer, uint32_t newSizeOfData, bool acknowledgement)
{
	// Free memory
	if (IsDataCloned && DataPointer != nullptr)
	{
		free(DataPointer);
	}

	SequenceNumber = newSequenceNumber;
	DataPointer = const_cast<uint8_t*>(newDataPointer);
	SizeOfData = newSizeOfData;
	IsAcknowledged = acknowledgement;
	IsDataCloned = false;
}

// Assign Packet Frame, with the data signature (that needs to be copied over)
void RUDPSocket::PacketFrame::Reassign(uint32_t newSequenceNumber, const std::vector<uint8_t>& newData, bool acknowledgement)
{
	// Free memory
	if (IsDataCloned && DataPointer != nullptr)
	{
		free(DataPointer);
	}

	// Transfer memory over to DataPointer
	SizeOfData = static_cast<uint32_t>(newData.size());
	uint8_t * clone = reinterpret_cast<uint8_t *>(malloc(SizeOfData));
	memcpy_s(clone, static_cast<size_t>(SizeOfData), newData.data(), static_cast<size_t>(SizeOfData));

	// Initialize Frame
	SequenceNumber = newSequenceNumber;
	DataPointer = clone;
	IsAcknowledged = acknowledgement;
	IsDataCloned = true;
}





// ======================================= RUDP Socket ==============================================

bool RUDPSocket::ReceiveValidPacket(uint8_t * buffer, uint32_t bufferSize, seq_num_t minSeq, uint32_t fragmentCount, RPacket& OutPacket)
{
	return ReceiveValidPacket(buffer, bufferSize, minSeq, fragmentCount, OutPacket, mMaxTimeout);
}

bool RUDPSocket::ReceiveValidPacket(uint8_t * buffer, uint32_t bufferSize, seq_num_t minSeq, uint32_t fragmentCount, RPacket & OutPacket, uint32_t maxTimeoutMS)
{
	std::string remoteAddress;
	uint16_t remotePort;
	uint32_t bytesReceived;
	bool isSuccess;

	for (;;)
	{
		try
		{	// Get Incoming Packets
			bytesReceived = UDPSocket::ReceiveFrom(reinterpret_cast<char *>(buffer), bufferSize, remoteAddress, remotePort, maxTimeoutMS);
		}
		catch (std::exception)
		{	// Revert Max Timeout Period
			throw;
		}

		if (bytesReceived == 0)
		{	// Connection Timed-out
			return false;
		}

		// Convert bytes to RUDP packet information
		isSuccess = OutPacket.Deserialize(buffer);

		if (!isSuccess)
		{	// Packet is not an RUDP Packet, ignore...
			Logger::PrintErrorF(__FILE__, ".... Not an RUDP Packet: expected id<%d>, actual id<%d>\n", OutPacket.RUDP_ID, OutPacket.Id());

			// Reset to Previous Client
			return false;
		}

		// RUDP Packet successfully received!
		{	// DEBUG: For Debugging Purposes only
			Logger::PrintF(__FILE__, "---- Rcvd [%5lld us]: ", CurrentTime());
			Logger::PrintF(__FILE__, BasicColor::YELLOW, "ack<%d> ", OutPacket.Ack());
			Logger::PrintF(__FILE__, BasicColor::CYAN, "seq<%d> ", OutPacket.Sequence());
			Logger::PrintF(__FILE__, BasicColor::YELLOW, "bit<%s> ", OutPacket.AckBitfield().ToString().c_str());
			Logger::PrintF(__FILE__, "msg_id<%d> frag<%d> bytes_rcvd<%d>\n", OutPacket.MessageId(), OutPacket.FragmentCount(), static_cast<uint32_t>(OutPacket.Buffer().size()));
		}

		if (mRemotePort != remotePort || mRemoteAddress != remoteAddress)
		{	// Random Packet received...
			Logger::PrintErrorF(__FILE__, ".... Received packet from different client. expected <%s:%d>  actual <%s:%d>\n"
				, mRemoteAddress.c_str(), mRemotePort, remoteAddress.c_str(), remotePort);
			continue;
		}
		else if (OutPacket.Sequence() < minSeq)
		{	// Old Packet received...
			Logger::PrintErrorF(__FILE__, ".... Received packet for old message, ignored...\n");
			continue;
		}
		else if (OutPacket.Sequence() >= minSeq + RPacket::NumberOfAcksPerPacket)
		{	// Packet outside of range received...
			Logger::PrintErrorF(__FILE__, ".... Received packet that does not fit within window, ignored...\n");
			continue;
		}
		else if ((fragmentCount != 0) && (OutPacket.Sequence() >= minSeq + fragmentCount))
		{	// Packet outside of range received...
			Logger::PrintErrorF(__FILE__, ".... Received packet for new message detected, ignored...\n");
			continue;
		}
		else
		{	// Packet in Valid Range. Return Valid Packet!
			return true;
		}
	}
}


bool RUDPSocket::SendPacket(const RPacket & packet)
{
	std::vector<uint8_t> packetData = std::move(packet.Serialize());
	uint32_t packetSize = static_cast<uint32_t>(packetData.size());
	bool isSent;

	//try
	//{	
	// Send Single-Data Packet
	isSent = UDPSocket::Send(reinterpret_cast<char *>(packetData.data()), packetSize);
	/*	TODO: Delete
	}
	catch (SocketException ex)
	{
		Logger::PrintErrorF(__FILE__, ".... sendto failed with error: %d %s\n", WSAGetLastError(), ex.what());
		throw;
	}*/

	{	// DEBUG: For Debugging Purposes only
		Logger::PrintF(__FILE__, "++++ Sent [%5lld us]: seq<%d> ack<%d> bit<%s> msg_id<%d> frag<%d> bytes_sent<%d>\n"
			, CurrentTime(), packet.Sequence(), packet.Ack(), packet.AckBitfield().ToString().c_str()
			, packet.MessageId(), packet.FragmentCount(), static_cast<uint32_t>(packet.Buffer().size()));
	}

	return isSent;
}

void RUDPSocket::MoveToNextMessage(uint32_t fragmentCount)
{
	mSequenceNumber += fragmentCount;
	mRemoteSequenceNumber += fragmentCount;
	++mMessageId;
}






RUDPSocket::RUDPSocket(uint16_t localPort, uint32_t maxTimeoutMS) : UDPSocket(localPort, maxTimeoutMS)
	, mSequenceNumber(0), mRemoteSequenceNumber(0), mMessageId(0) {}

bool RUDPSocket::Send(const void * buffer, uint32_t bufferSize)
{
	// Handle Special Cases
	if (bufferSize == 0)
	{
		throw SocketException("Buffer Size cannot be zero.");
	}
	else if (buffer == nullptr)
	{
		throw SocketException("Null Pointer reference for buffer.");
	}

	// Convert Size to Integer Data
	int32_t currentSizeOfData = static_cast<int32_t>(bufferSize);
	if (currentSizeOfData < 0)
	{	// Size overflowed, size is too large, don't even send
		throw SocketException("Size of Buffer is too large.");
	}

	// Variables
	uint32_t totalBytesSent = 0;
	RPacket packet;
	bool isSuccess;

	// ======= Send the Data in Packets =====

	// Declare this as a new message
	//++mMessageId;

	{	// DEBUG: For Debugging Purposes only
		CurrentTime(true);
		Logger::PrintF(__FILE__, "==== Begin Sending to %s:%d: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d>\n"
			, GetRemoteAddress().c_str(), GetRemotePort(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxTimeout);
	}

	// Create Packet Buffer (with RAII using unique_ptr)
	//std::unique_ptr<uint8_t[sBufferSize]> packetBuffer = std::unique_ptr<uint8_t[sBufferSize]>();
	std::unique_ptr<uint8_t> packetBuffer = std::unique_ptr<uint8_t>(reinterpret_cast<uint8_t*>(malloc(sBufferSize)));

	// Make copies of the state of the Local and Remote Sequence
	// TODO: What it said...

	// Calculate Number and size of Fragments (using Ceiling Function)
	int numberOfFragments = ((bufferSize - 1) / sMaximumDataSize) + 1;
	int sizeOfPacketToSend = ((bufferSize - 1) / numberOfFragments) + 1;

	// Track the Data being Sent by Fragments
	const uint8_t * currentDataLocation = reinterpret_cast<const uint8_t *>(buffer);
	int numOfBytesToSend;

	bool isMostRecentAcknowledged = false;	// Initialize the Two Acknowledge State

	ackbitfield_t localAcknowledgedBitfield(ackbitfield_t::AllTrue);		// Tracks the Received Packets from the Receiver and sets the AckBitfield accordingly
	ackbitfield_t remoteAcknowledgedBitfield(ackbitfield_t::AllTrue);		// Tracks the Sent Packets Acknowledged by the Receiver
	
	
	// Initialize "Sliding Window" characteristics
	SlidingWindow window;								// Sliding Window used to track the state of the Sent Packets
	PacketFrame currentFrame;
	PacketFrame poppedFrame;

	// Initialize Acknowledgement characteristics
	uint32_t lastReceivedRemoteSequenceNumber = mRemoteSequenceNumber - 1;

	// Other Variables
	seq_num_t currentSequenceNumber = mSequenceNumber;

	// Send with fragment data
	for (int fragment = 1; fragment <= numberOfFragments; ++fragment)
	{
		// ====================================================================================
		// ============================== Send Packet of Data =================================
		// ====================================================================================

		// Get the Minimum Between the current bytes left and the largest sized packet to send
		numOfBytesToSend = (currentSizeOfData < sizeOfPacketToSend) ? currentSizeOfData : sizeOfPacketToSend;

		// Create Packet Data (of appropriate size)
		packet.Initialize(currentSequenceNumber, lastReceivedRemoteSequenceNumber + 1, remoteAcknowledgedBitfield, mMessageId, numberOfFragments, currentDataLocation, numOfBytesToSend);
		SendPacket(packet);

		// Create Current Packet Frame
		currentFrame.Reassign(currentSequenceNumber, currentDataLocation, numOfBytesToSend, false);

		// Update Sliding Window
		isSuccess = window.PushBack(currentFrame, &poppedFrame);

		// Check if Sliding Window pushed out a value
		if (!isSuccess)
		{	// Handle the packet that did not go through
			if(poppedFrame.IsAcknowledged == false)
			{	// Packet was never acknowledged

				// TODO: Handle packet loss better...
				//packetsLost.push_back(poppedFrame);
				//MoveToNextMessage(numberOfFragments);

				Logger::PrintF(__FILE__, "==== Sending Failed. No acknowledgement received during send before escaping window.\n");

				Logger::PrintF(__FILE__, "==== Final State: [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> msg_size<%d> bytes_sent<%d>\n"
					, CurrentTime(), currentSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxTimeout, bufferSize, totalBytesSent);

				return false;
			}
		}

		// Update Acknowledgement Bits (Shift bits same as Sliding Window)
		localAcknowledgedBitfield.bitfield <<= 1;
		localAcknowledgedBitfield.bit00 = isMostRecentAcknowledged;

		// Update Sequence Number
		++currentSequenceNumber;
		isMostRecentAcknowledged = false;

		// Update bytes left to send
		currentDataLocation += numOfBytesToSend;
		currentSizeOfData -= numOfBytesToSend;
		totalBytesSent += static_cast<uint32_t>(packet.Buffer().size());

		// ====================================================================================
		// ============================ Receive Acknowledgement ===============================
		// ====================================================================================

		// Check for any Acknowledgements Received
		for (;;)
		{
			// Get Incoming Packet
			// Only wait on the second-to-last and last packet sent
			isSuccess = ReceiveValidPacket(reinterpret_cast<uint8_t*>(packetBuffer.get()), sBufferSize
										, mRemoteSequenceNumber, numberOfFragments, packet
										, ((fragment >= numberOfFragments - 1) ? (mMaxTimeout) : (1U)));

			if (isSuccess)
			{	// Valid RUDP Packet Received!
				// ============================================================
				// =================== Handle Sequence Number =================
				// ============================================================

				int32_t packetOffset = DiffFromSeq1ToSeq2(lastReceivedRemoteSequenceNumber, packet.Sequence());

				// Check if the Packet received is more recent
				if (packetOffset > 0)
				{	// Update last ack received to this packet's sequence

					// DEBUG: For Debug Purposes Only
					if (packetOffset != 1)
					{
						Logger::PrintErrorF(__FILE__, ".... Packet Re-ordering Detected! expected<%d> actual<%d>\n", lastReceivedRemoteSequenceNumber + 1, packet.Sequence());
					}

					// Create Bitmask to check all the ack_bits that will be shifted off (starting from the end bit31)
					uint32_t bit_mask = ~(0U);	// All start as true (1)
					bit_mask >>= packetOffset;	// Make the last N bits equal false (0)... this will be our mask to check if the last N bits are true

					if (~(remoteAcknowledgedBitfield.bitfield | bit_mask) != 0U)
					{	// If the last N bits do not all equal true (N = packetOffset), then at least one packet shifted was not acknowledged

						// TODO: Handle unacknowledged packet...
						Logger::PrintErrorF(__FILE__, ".... Packet Loss detected, but currently ignoring...\n");
					}

					// Include all 32 results of acknowledged bitfield received (using OR gate logic)
					remoteAcknowledgedBitfield.bitfield <<= packetOffset;
					remoteAcknowledgedBitfield.Set(packetOffset - 1, true);	// The bit for the last Received Remote Sequence Number needs to get set!

					lastReceivedRemoteSequenceNumber = packet.Sequence();		// Update to latest remote sequence number
				}
				else if(packetOffset < 0)
				{	// Set Received Packet's Ackbit

					packetOffset = -packetOffset;

					if (packetOffset >= RPacket::NumberOfAcksPerPacket)
					{	// DEBUG: For Debug Purposes Only
						Logger::PrintErrorF(__FILE__, ".... Ack Received, but not able to Set value...\n");
					}
					else
					{	// Set the ack_bit for the respective acknowledged number received
						remoteAcknowledgedBitfield.Set(packetOffset - 1, true);
							
						// Include all 32 results of acknowledged bitfield received (using OR gate logic)
						//recvdPacketsAckBitfield.bitfield |= (packet.AckBitfield().bitfield << packetOffset);
					}
				}

				// ============================================================
				// ============= Handle Ack Number and Ack-Bitfield ===========
				// ============================================================

				// This is the most important part of the sending process.
				// This test must pass in order for the send to be considered successful.


				// Set the Sent Packet's Ack Bitfield according to the Ack and AckBitfield received
				// (recall Seq Number is 1 ahead of what's sent, and Ack Numbers await for the Seq Number ahead)
				packetOffset = DiffFromSeq1ToSeq2(packet.Ack(), currentSequenceNumber);
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
						localAcknowledgedBitfield.Set(packetOffset - 1, true);
					}

					// Set the received Ack Number
					// Include all 32 results of acknowledged bitfield received (using OR gate logic)
					localAcknowledgedBitfield.bitfield |= (packet.AckBitfield().bitfield << packetOffset);	// Set All Acks in received AckBitfield
				}
			}
			else if(fragment >= numberOfFragments - 1)
			{	// RUDP Packet not received
				// Assert that there's no Connection Time-Outs

				//MoveToNextMessage(numberOfFragments);

				Logger::PrintF(__FILE__, "==== Sending Failed. No acknowledgement received during send. Connection Timed-Out\n");

				Logger::PrintF(__FILE__, "==== Final State: [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> msg_size<%d> bytes_sent<%d>\n"
					, CurrentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxTimeout, bufferSize, totalBytesSent);
				return false;
			}

			// DEBUG: For Debugging Purposes only
			Logger::PrintF(__FILE__, BasicColor::YELLOW,"          Internal State:   local_ack_bit:  (%d) %s\n"
				, currentSequenceNumber, localAcknowledgedBitfield.ToString().c_str());
			// DEBUG: For debugging purpose only
			Logger::PrintF(__FILE__, BasicColor::CYAN, "          Internal State:   remote_ack_bit: (%d) %s\n"
				, lastReceivedRemoteSequenceNumber, remoteAcknowledgedBitfield.ToString().c_str());

			// ============== Check Condition to Continue Receiving Packets =================
			if (fragment >= numberOfFragments - 1)
			{
				Logger::PrintF(__FILE__, "Exit Condition: isMostRecent=%s localAckBit=%s remoteAckBit=%s expected<%d> current<%d>\n"
					,(isMostRecentAcknowledged) ?"true":"false"
					, localAcknowledgedBitfield.IsAll(true) ? "true" : "false"
					, remoteAcknowledgedBitfield.IsAll(true) ? "true" : "false"
					, mRemoteSequenceNumber + fragment, lastReceivedRemoteSequenceNumber + 1);
			}


			if (fragment < numberOfFragments - 1)
			{	// Still fragments left to send, keep sending
				break;
			}
			else if (isMostRecentAcknowledged && localAcknowledgedBitfield.IsAll(true)
				&& lastReceivedRemoteSequenceNumber + 1 == mRemoteSequenceNumber + fragment
				&& remoteAcknowledgedBitfield.IsAll(true))
			{	// The Second-to-Last fragment: With all acknowledgements received send last fragment packet with all acknowledgements
				// The Last fragment: With all acknowledgements received, exit the loop successful!

				break;
			}
		}
	}

	// Successful Sending, update state.
	//mRemoteSequenceNumber = lastReceivedRemoteSequenceNumber + 1;	// This might work, use other just in case
	MoveToNextMessage(numberOfFragments);
	
	{	// DEBUG: For Debug Purposes Only!
		Logger::PrintF(__FILE__, "==== Successful Sending\n");
		Logger::PrintF(__FILE__, "==== Final State: [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> msg_size<%d> bytes_sent<%d>\n"
			, CurrentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxTimeout, bufferSize, totalBytesSent);
	}
	assert(currentSizeOfData == 0);				// Number of data left to transmit should be zero!

	// Return Results
	return true;
}

uint32_t RUDPSocket::Receive(void * OutBuffer, uint32_t bufferSize)
{
	// Create Packet Buffer (with RAII using unique_ptr)
	std::unique_ptr<uint8_t> packetBuffer = std::unique_ptr<uint8_t>(reinterpret_cast<uint8_t*>(malloc(sBufferSize)));

	// Initialize the Two Acknowledge State
	uint32_t lastSentPacketAcknowledged = mSequenceNumber - 1;
	ackbitfield_t localAcknowledgedBitfield(ackbitfield_t::AllTrue);				// Tracks the Sent Packets Acknowledged by the Receiver
	ackbitfield_t remoteAcknowledgedBitfield(ackbitfield_t::AllTrue);				// Tracks the Received Packets from the Receiver and sets the AckBitfield accordingly
	
	// Initialize "Sliding Window" characteristics
	SlidingWindow window;
	PacketFrame currentFrame;
	PacketFrame poppedFrame;
	uint32_t oldestRSNfromWindow = mRemoteSequenceNumber - 1;
	uint32_t newestRSNfromWindow = mRemoteSequenceNumber - 1;

	// Track the Data being Sequenced
	std::vector<PacketFrame> sequencedData;

	uint32_t receivedRemoteSequenceNumber;
	uint32_t currentSequenceNumber = mSequenceNumber;

	bool isFirstPacket = true;
	int32_t packetOffset;
	bool isSuccess;
	RPacket packet;
	uint32_t fragmentCount = 0;

	for (;;)
	{
		// ====================================================================================
		// ================================== Receive RPacket =================================
		// ====================================================================================

		isSuccess = ReceiveValidPacket(reinterpret_cast<uint8_t*>(packetBuffer.get()), sBufferSize
			, mRemoteSequenceNumber, fragmentCount, packet, mMaxTimeout);
		
		if (!isSuccess)
		{	// Connection Timed-Out
			if (!isFirstPacket)
			{	// Adjust expected sequence number
				//MoveToNextMessage(fragmentCount);

				Logger::PrintF(__FILE__, "==== Receiving Failed: Connection Timed-Out ====\n");
				Logger::PrintF(__FILE__, "==== Final State [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> bytes_rcvd<0>\n"
					, CurrentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxTimeout);
			}

			return false;
		}

		// Calculate how many packets ahead this packet is
		receivedRemoteSequenceNumber = packet.Sequence();
		packetOffset = DiffFromSeq1ToSeq2(oldestRSNfromWindow, receivedRemoteSequenceNumber);
		assert(oldestRSNfromWindow + packetOffset == receivedRemoteSequenceNumber);			// Assert the DiffFromFunction works

																							// Determine if Packet is within a valid range
		if (packetOffset < 0 || packetOffset >= RPacket::NumberOfAcksPerPacket)
		{
			Logger::PrintErrorF(__FILE__, ".... RUDP Packet is not within a valid range: current<%d>, received<%d>, dif<%d>\n"
				, oldestRSNfromWindow, receivedRemoteSequenceNumber, packetOffset);

			continue;
		}

		if (receivedRemoteSequenceNumber < mRemoteSequenceNumber)
		{	// Old Packet received...
			Logger::PrintErrorF(__FILE__, ".... Packet for new message detected, but ignored...\n");
			continue;
		}
		else if (!isFirstPacket && receivedRemoteSequenceNumber >= mRemoteSequenceNumber + fragmentCount)
		{	// Packet outside of range received...
			Logger::PrintErrorF(__FILE__, ".... Packet for new message detected, but ignored...\n");
			continue;
		}


		// Assert that the message is correct
		assert(isFirstPacket || mMessageId == packet.MessageId());

		// ============================== Handle First Packet =================================

		// Handle Case of first packet of message received
		if (isFirstPacket)
		{
			fragmentCount = packet.FragmentCount();

			isFirstPacket = false;
			assert(static_cast<uint32_t>(packetOffset) <= fragmentCount);	// Assert that the packet received is not more than the fragment count

																			// Declare this as a new message
			//++mMessageId;

			{	// DEBUG: For Debugging Purposes only
				CurrentTime(true);
				Logger::PrintF(__FILE__, "==== Begin Receiving from %s:%d : seq<%d> remote_seq<%d> msg_id<%d> frag<%d> bytes_rcvd<%d> max_time<%d>\n"
					, mRemoteAddress.c_str(), mRemotePort, mSequenceNumber, mRemoteSequenceNumber, mMessageId
					, (int)fragmentCount, packet.Buffer().size(), mMaxTimeout);
			}
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

			// DEBUG: For Debug Purposes Only
			if (packetOffset != 1)
			{
				Logger::PrintErrorF(__FILE__, ".... Packet Re-ordering Detected! expected<%d> actual<%d>\n", newestRSNfromWindow + 1, receivedRemoteSequenceNumber);
			}

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
				remoteAcknowledgedBitfield.bitfield <<= 1;
				remoteAcknowledgedBitfield.bit00 = window.IsEmpty() || window[0].IsAcknowledged;

				// Update Sliding Window
				isSuccess = window.PushBack(currentFrame, &poppedFrame);

				// Check if Sliding Window pushed out a value
				if (!isSuccess)
				{	// Handle the packet that did not go through
					if (poppedFrame.IsAcknowledged == false)
					{	// Packet was never acknowledged, return unsuccessful submission

						// TODO: Handle packet loss case better...
						Logger::PrintErrorF(__FILE__, ".... Packet Loss Detected! Receive Cannot complete. . .\n");

						// Adjust expected sequence number
						mSequenceNumber += fragmentCount;
						mRemoteSequenceNumber += fragmentCount;

						Logger::PrintF(__FILE__, "==== Final State [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> bytes_rcvd<0>\n"
							, CurrentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxTimeout);
						return false;
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
		else if (packetOffset < 0)
		{	// Packet is older, update window accordingly

			assert(packetOffset >= (0 - RPacket::NumberOfAcksPerPacket));	// Assert OldestRNS clamped packets properly

			int32_t index = (-1) - packetOffset;//(RPacket::NumberOfAcksPerPacket - 1) + packetOffset;

												// Check if packet is acknowledged
			if (!remoteAcknowledgedBitfield[index])
			{	// Packet not acknowledged, update Packet Frame state
				remoteAcknowledgedBitfield.Set(index, true);
				PacketFrame& frame = window[index + 1];

				// Save Buffer in Sequence
				frame.Reassign(receivedRemoteSequenceNumber, packet.Buffer(), true);
			}
			else
			{	// Packet already acknowledged, redundant packet ignored...
				Logger::PrintErrorF(__FILE__, ".... Redundant Packet Detected: seq<%d>\n", receivedRemoteSequenceNumber);
				continue;
			}
		}
		else
		{	// Packet is currently newest packet received prior, redunant packet ignored...
			assert(!window.IsEmpty() || window.Back().IsAcknowledged);

			Logger::PrintErrorF(__FILE__, ".... Redundant Packet Detected: seq<%d>\n", receivedRemoteSequenceNumber);
			continue;
		}

		// DEBUG: For Debugging Purposes only
		Logger::PrintF(__FILE__, BasicColor::YELLOW, "          Internal State:   local_ack_bit:  (%d) %s\n"
			, lastSentPacketAcknowledged, localAcknowledgedBitfield.ToString().c_str());

		// DEBUG: For debugging purpose only
		Logger::PrintF(__FILE__, BasicColor::CYAN, "          Internal State:   remote_ack_bit: (%d) %s\n"
			, receivedRemoteSequenceNumber, remoteAcknowledgedBitfield.ToString().c_str());


		// ============================ Handling Acknowledgements =============================
		// DEBUG:
		if (newestRSNfromWindow == mRemoteSequenceNumber + fragmentCount - 1)
		{
			fragmentCount = fragmentCount;
		}

		if (isFirstPacket)
		{
			assert(localAcknowledgedBitfield.bitfield == ~(0U));	// The sent ack_bitfield must all be initialized to true to avoid confusion...
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

				if (~(localAcknowledgedBitfield.bitfield | bit_mask) != 0U)
				{	// If the last N bits do not all equal true (N = packetOffset), then at least one packet shifted was not acknowledged

					// TODO: Handle unacknowledged packet...
					Logger::PrintErrorF(__FILE__, ".... Packet Loss detected, but currently ignoring...\n");
				}

				// Include all 32 results of acknowledged bitfield received (using OR gate logic)
				localAcknowledgedBitfield.bitfield <<= packetOffset;
				localAcknowledgedBitfield.bitfield |= packet.AckBitfield().bitfield;

				// Update last packet acknowledged to this ack number
				lastSentPacketAcknowledged = packet.Ack();
			}
			else if (packetOffset < 0)
			{
				packetOffset = -packetOffset;

				if (packetOffset >= RPacket::NumberOfAcksPerPacket)
				{	// DEBUG: For Debug Purposes Only
					Logger::PrintErrorF(__FILE__, ".... Ack Received, but not able to Set value...\n");
				}
				else
				{	// Set the ack_bit for the respective acknowledged number received
					localAcknowledgedBitfield.Set(packetOffset - 1, true);

					// Include all 32 results of acknowledged bitfield received (using OR gate logic)
					localAcknowledgedBitfield.bitfield |= (packet.AckBitfield().bitfield << packetOffset);
				}
			}
			else
			{
				// Include all 32 results of acknowledged bitfield received (using OR gate logic) in case of update...
				localAcknowledgedBitfield.bitfield |= packet.AckBitfield().bitfield;
				Logger::PrintErrorF(__FILE__, ".... Redundant acknowledgement ignored: ack<%d>\n", packet.Ack());
			}
		}


		// ====================================================================================
		// =========================== Send / Return Acknowledgement ==========================
		// ====================================================================================

		// Send Acknowledgement Information
		packet.Initialize(currentSequenceNumber, newestRSNfromWindow + 1, remoteAcknowledgedBitfield, mMessageId, 1, std::vector<uint8_t>());
		isSuccess = SendPacket(packet);
		assert(isSuccess);		// No reason for this to fail...

								// Update Sequence Number
		++currentSequenceNumber;

		// DEBUG: For Debug Purposes Only
		if (currentSequenceNumber >= mSequenceNumber + fragmentCount)
		{
			Logger::PrintF(__FILE__, "Exit Condition: localAckBit=%s remoteAckBit=%s expected_remote<%d> current<%d>   expected_seq<%d> current<%d>\n"
				, localAcknowledgedBitfield.IsAll(true) ? "true" : "false"
				, remoteAcknowledgedBitfield.IsAll(true) ? "true" : "false"
				, mRemoteSequenceNumber + fragmentCount
				, newestRSNfromWindow + 1
				, currentSequenceNumber
				, lastSentPacketAcknowledged + 1);
		}

		// Check if all acks received...
		if ((newestRSNfromWindow + 1 == mRemoteSequenceNumber + fragmentCount) && (remoteAcknowledgedBitfield.IsAll(true))
			&& (lastSentPacketAcknowledged + 1 == currentSequenceNumber) && (localAcknowledgedBitfield.IsAll(true)))
		{	// True perfection has been reached in transmitting this message. Break free!
			// Adjust expected sequence number

			break;
		}
	}
	
	// Message Received successfully
	MoveToNextMessage(fragmentCount);

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

	if (totalBytesReceived > bufferSize)
	{	// TODO: Handle this case better!
		Logger::PrintF(__FILE__, "==== Receive Failed. Bytes received waaay too large: max<%d> rcvd<%d>\n", bufferSize, totalBytesReceived);

		Logger::PrintF(__FILE__, "==== Final State [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> bytes_rcvd<%d>\n"
			, CurrentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxTimeout, totalBytesReceived);
		return false;
	}
	else if (totalBytesReceived <= 0)
	{
		Logger::PrintF(__FILE__, "==== Receive Failed. Bytes received is bad: rcvd<%d>\n", totalBytesReceived);

		Logger::PrintF(__FILE__, "==== Final State [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> bytes_rcvd<%d>\n"
			, CurrentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxTimeout, totalBytesReceived);

		return false;
	}
	else
	{	// DEBUG: For Debugging Purposes only
		Logger::PrintF(__FILE__, "==== Successful Receiving\n");

		Logger::PrintF(__FILE__, "==== Final State [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> bytes_rcvd<%d>\n"
			, CurrentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxTimeout, totalBytesReceived);
	}


	// Copy Sequenced Data into Buffer
	char * CurrentBufferLocation = reinterpret_cast<char*>(OutBuffer);
	for (auto& frame : sequencedData)
	{
		memcpy(CurrentBufferLocation, frame.DataPointer, frame.SizeOfData);
		CurrentBufferLocation += frame.SizeOfData;
	}

	// Return Successful Receipt of Data
	return totalBytesReceived;
}

bool RUDPSocket::Connect(const std::string& remoteAddress, uint16_t remotePort)
{
	/** Perform Three-way Hand-shaking
	*	This works as follows
	*	 - Client sends message with SeqNum X
	*	 - Server receives message and returns message with SeqNum Y and AckNum X+1
	*	 - Client receives, checks AckNum for X+1, and returns message with AckNum Y+1
	*	 - Server receives message, checks AckNum for Y+1, and
	*	Returning the Num+1 tells the receiving end that the number was recognized.
	*/

	// Generate Initial Sequence Number
	mSequenceNumber = rand();

	// Generate Request Packet
	std::vector<uint8_t> acknowledgePacket = RPacket::SerializeInstance(mSequenceNumber, 0, 0, 0, 0, std::vector<uint8_t>());

	// Begin Establishing Connection
	RPacket data;
	uint32_t packetSize;
	uint32_t bytesReceived;
	char buffer[512];

	std::string fromAddress = remoteAddress;
	unsigned short fromPort = remotePort;
	bool isSuccess;
	static int ADDR_LEN = sizeof(struct sockaddr_in);
	for (;;)
	{
		// Send Request Packet
		packetSize = static_cast<uint32_t>(acknowledgePacket.size());
		try
		{
			SendTo(reinterpret_cast<char *>(acknowledgePacket.data()), packetSize, remoteAddress, remotePort);
		}
		catch (SocketException ex)
		{
			Logger::PrintF(__FILE__, "sendto failed with error: %s\n", ex.what());
			return false;
		}

		// Begin Timing (Record Time it was Sent)
		std::chrono::high_resolution_clock::time_point timeSentToServer = std::chrono::high_resolution_clock::now();
		Logger::PrintF(__FILE__, "Sending to Server: ip<%s> port<%d>\n", remoteAddress.c_str(), remotePort);
		Logger::PrintF(__FILE__, "Establishing Connection: Sending seq <%d> ack<%d>\n", mSequenceNumber, 0);

		for (;;)
		{
			// Get Incoming Packets
			bytesReceived = 512;
			isSuccess = ReceiveFrom(buffer, bytesReceived, fromAddress, fromPort, mMaxTimeout);

			// Check if Packet is Received
			if (isSuccess)
			{	// Packet has been received!

				// Check if packet's address is from server
				if (remoteAddress != fromAddress)
				{	// Packet is not from the server, ignore...
					Logger::PrintF(__FILE__, "RUDP Packet is not from Server: expected<%s> actual<%s>\n", remoteAddress.c_str(), fromAddress.c_str());
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
				return false;
			}
		}

		// Check if Acknowledge Number is your Sequence Number
		if (data.Ack() != mSequenceNumber + 1)
		{	// Bad Acknowledgement number, send again!
			Logger::PrintF(__FILE__, "Bad Acknowledgement number: expected<%d> actual<%d>\n", mSequenceNumber + 1, data.Ack());
			continue;
		}

		// Acknowledgement Successful!
		Logger::PrintF(__FILE__, "Acknowledged Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());
		break;
	}

	// Send Acknowledgement to Complete Three-Way Handshake
	mSequenceNumber += 1;
	mRemoteSequenceNumber = data.Sequence() + 1;
	acknowledgePacket = RPacket::SerializeInstance(mSequenceNumber, mRemoteSequenceNumber, 0, 0, 0, std::vector<uint8_t>());

	// Send Acknowledged Packet
	packetSize = static_cast<uint32_t>(acknowledgePacket.size());
	try
	{
		SendTo(reinterpret_cast<char *>(acknowledgePacket.data()), packetSize, fromAddress, fromPort);
	}
	catch (SocketException ex)
	{
		Logger::PrintF(__FILE__, "sendto failed with error: %s\n", ex.what());
		return false;
	}


	// At this point the connection is established
	Logger::PrintF(__FILE__, "Acknowledging Connection: Sending  seq <%d> ack<%d>\n", mSequenceNumber, mRemoteSequenceNumber);

	// Next message that will be sent should be one sequence number higher
	mSequenceNumber += 1;

	// Set the Port to connect with from now on
	mRemoteAddress = fromAddress;
	mRemotePort = fromPort;
	return true;
}

//bool RUDPSocket::SendTo(const void* , uint32_t , const std::string& , uint16_t) { return false; }
//bool RUDPSocket::ReceiveFrom(void* , uint32_t& , std::string& , uint16_t& , uint32_t) { return false; }

RUDPServerSocket::RUDPServerSocket(std::uint16_t listenPort, std::uint32_t maxTimeoutMS) : UDPSocket(listenPort, maxTimeoutMS)
	, mAvailablePort(listenPort + 1), mMaxConnectionTimeOut(maxTimeoutMS), mAcknowledgeTable() {}

RUDPSocket* RUDPServerSocket::Accept()
{
	static int32_t ADDR_LEN = sizeof(struct sockaddr_in);

	uint32_t bytesReceived;
	char buffer[512];
	uint32_t seqNum;
	uint32_t ackNum;
	RPacket data;
	bool isSuccess;

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
			isSuccess = ReceiveFrom(buffer, bytesReceived, clientAddress, clientPort, mMaxConnectionTimeOut);

			// Check if packet is received
			if (isSuccess)
			{	// Packet has been received!

				// Convert bytes to RUDP packet information
				isSuccess = data.Deserialize(reinterpret_cast<uint8_t *>(buffer));

				if (!isSuccess)
				{	// Packet is not an RUDP Packet
					Logger::PrintF(__FILE__, "Received non-RUDP Packet on Listening Port %d: id<%X>\n", GetLocalPort(), data.Id());
				}
			}
		}
		else
		{
			Logger::PrintF(__FILE__, "Ignore listen\n");
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
				Logger::PrintF(__FILE__, "New Client found! ip<%s> port<%d>\n", clientAddress.c_str(), clientPort);
				isClientFound = true;

				Logger::PrintF(__FILE__, "Establishing Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());

				// Create RPacket to send...
				seqNum = rand();
				ackNum = data.Sequence() + 1U;
				std::vector<uint8_t> acknowledgePacket = RPacket::SerializeInstance(seqNum, ackNum, 0, 0, 0, std::vector<uint8_t>());

				// Create new socket to communicate with client
				std::shared_ptr<RUDPSocket> clientSocket(std::make_shared<RUDPSocket>(mAvailablePort));
				++mAvailablePort;

				try
				{
					clientSocket->SendTo(acknowledgePacket.data(), static_cast<int>(acknowledgePacket.size()), clientAddress, clientPort);
				}
				catch (SocketException ex)
				{
					Logger::PrintF(__FILE__, "sendto failed with error: %s\n", ex.what());
				}

				// Store acknowledgement information in table
				Logger::PrintF(__FILE__, "Establishing Connection: Sending  seq <%d> ack<%d>\n", seqNum, ackNum);
				mAcknowledgeTable.push_back(PendingClientsT(clientSocket, clientAddress, clientPort, seqNum, ackNum));
			}
		}
		else
		{	// RUDP Packet not received

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
						Logger::PrintF(__FILE__, "Received non-RUDP Packet on Port %d: id<%X>\n", client.socket->GetLocalPort(), data.Id());
					}
				}

				if(isSuccess)
				{	// Client has been Acknowledged before...

					// Check if Acknowledgement is correct
					if (data.Ack() == client.seqNumSent + 1U)
					{	// Successful Acknowledgement from Client!
						Logger::PrintF(__FILE__, "Acknowledged Connection: Received seq <%d> ack<%d>\n", data.Sequence(), data.Ack());

						// Create RUDP Stream
						client.socket->mRemoteAddress = client.address;
						client.socket->mRemotePort = client.port;
						client.socket->mMaxTimeout = mMaxConnectionTimeOut;
						client.socket->mSequenceNumber = client.seqNumSent + 1;
						client.socket->mRemoteSequenceNumber = client.ackNumRecvd + 1;

						// Remove from Acknowledged Clients Table
						removeIndices.push_back(index);

						// Connection Established
						Logger::PrintF(__FILE__, "Connection Established! Client Port is %d. Next Available Port is %d\n", client.socket->GetLocalPort(), mAvailablePort);

						// Add to the list of Clients
						mClients.push_back(client.socket);

						// Return the client
						return client.socket.get();
					}
					else
					{	// Client has a bad acknowledgent number
						Logger::PrintF(__FILE__, "Received Bad Acknowledgement Number: seq<%d> ack<%d>\n", data.Sequence(), data.Ack());
					}
				}
				else
				{	// Connection timed-out for this acknowledgement
					Logger::PrintF(__FILE__, "Connection Timed-Out for Client %s:%d\n", client.address.c_str(), client.port);

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

			// Set if client exist
			isClientFound = (mAcknowledgeTable.size() != 0);
		}
	}
}

uint16_t RUDPServerSocket::GetListeningPort()
{
	return GetLocalPort();
}