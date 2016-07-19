#include "stdafx.h"
#include "RUDPStream.h"
#include "RPacket.h"
#include <assert.h>
#include <time.h>

#include "UDPSocket.h"
#include "SocketException.h"
#include "Logger.h"








RUDPStream::PacketFrame::PacketFrame() : SequenceNumber(0), DataPointer(nullptr), SizeOfData(0), IsAcknowledged(false), IsDataCloned(false) {}

RUDPStream::PacketFrame::PacketFrame(const PacketFrame& other)\
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

RUDPStream::PacketFrame& RUDPStream::PacketFrame::operator=(const PacketFrame& rhs)
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

RUDPStream::PacketFrame::PacketFrame(PacketFrame&& other) : SequenceNumber(other.SequenceNumber), IsAcknowledged(other.IsAcknowledged)
, DataPointer(other.DataPointer), SizeOfData(other.SizeOfData), IsDataCloned(other.IsDataCloned)
{
	other.DataPointer = nullptr;
	other.SizeOfData = 0;
	other.IsDataCloned = false;
}

RUDPStream::PacketFrame& RUDPStream::PacketFrame::operator=(PacketFrame&& rhs)
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

RUDPStream::PacketFrame::~PacketFrame()
{	// Free memory
	if (IsDataCloned && DataPointer != nullptr)
	{
		//free(DataPointer);
	}
}

// Assign Packet Frame as Bad
void RUDPStream::PacketFrame::Reassign(uint32_t newSequenceNumber)
{
	Reassign(newSequenceNumber, nullptr, 0, false);
}

// Assign Packet Frame, with the data signature
void RUDPStream::PacketFrame::Reassign(uint32_t newSequenceNumber, const uint8_t * newDataPointer, uint32_t newSizeOfData, bool acknowledgement)
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
void RUDPStream::PacketFrame::Reassign(uint32_t newSequenceNumber, const std::vector<uint8_t>& newData, bool acknowledgement)
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













RUDPStream::RUDPStream(const std::shared_ptr<UDPSocket>& socket, std::string toAddress, unsigned short toPort
	, const uint32_t& senderSequenceNumber, const uint32_t& receiverSequenceNumber, uint32_t maxConnectionTimeOut)
	: mSocket(socket), mToAddress(toAddress), mToPort(toPort), mSequenceNumber(senderSequenceNumber)
	, mRemoteSequenceNumber(receiverSequenceNumber), mMaxConnectionTimeOut(maxConnectionTimeOut), mBuffer(new uint8_t[sBufferSize])
{
	/*
	// Before beginning the process, we must make the recvfrom non-blocking
	// This is to check on a time-out period to deliver the message again.
	DWORD NON_BLOCKING = TRUE;
	if (ioctlsocket(mSocket, FIONBIO, &NON_BLOCKING) != 0)
	{
		Logger::PrintF(__FILE__, "RUDPStream failed to set non-blocking\n");
	}
	*/
}

RUDPStream RUDPStream::InvalidStream()
{
	RUDPStream invalid(std::shared_ptr<UDPSocket>(), "", 0, 0,0,0);
	return invalid;
}

RUDPStream::RUDPStream(const RUDPStream & other) : mSocket(other.mSocket), mToAddress(other.mToAddress), mToPort(other.mToPort)
	, mMaxConnectionTimeOut(other.mMaxConnectionTimeOut), mBuffer(new uint8_t[sBufferSize])
{
	memcpy_s(mBuffer, sBufferSize, other.mBuffer, sBufferSize);
}

RUDPStream & RUDPStream::operator=(const RUDPStream & rhs)
{
	if (this != &rhs)
	{
		mSocket = rhs.mSocket;
		mToAddress = rhs.mToAddress;
		mToPort = rhs.mToPort;
		mMaxConnectionTimeOut = rhs.mMaxConnectionTimeOut;
		memcpy_s(mBuffer, sBufferSize, rhs.mBuffer, sBufferSize);
	}
	return *this;
}

RUDPStream::RUDPStream(RUDPStream && other)	: mSocket(std::move(other.mSocket)), mToAddress(std::move(other.mToAddress))
	, mToPort(std::move(other.mToPort)), mMaxConnectionTimeOut(std::move(other.mMaxConnectionTimeOut)), mBuffer(other.mBuffer) 
{
	other.mBuffer = nullptr;
}

RUDPStream & RUDPStream::operator=(RUDPStream && rhs)
{
	if (this != &rhs)
	{
		mSocket = std::move(rhs.mSocket);
		mToAddress = std::move(rhs.mToAddress);
		mToPort = std::move(rhs.mToPort);
		mMaxConnectionTimeOut = std::move(rhs.mMaxConnectionTimeOut);
		mBuffer = rhs.mBuffer;

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

const uint64_t currentTime(bool reset = false)
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

bool RUDPStream::ReceiveValidPacket(uint32_t & OutBytesReceived, RPacket & OutPacket, int32_t maxTimeOutMS)
{
	uint32_t bytesReceived;
	bool isSuccess;
	std::string sourceAddress;
	unsigned short sourcePort;

	// Get Incoming Packets
	bytesReceived = sBufferSize;
	isSuccess = mSocket->ReceiveFrom(reinterpret_cast<char *>(mBuffer), bytesReceived, sourceAddress, sourcePort, maxTimeOutMS);

	// Record Received Bytes
	OutBytesReceived = (bytesReceived > 0) ? static_cast<uint32_t>(bytesReceived) : 0U;

	// Check if Packet is Received
	if (isSuccess)
	{	// Packet has been received!

		// Convert bytes to RUDP packet information
		isSuccess = OutPacket.Deserialize(mBuffer);

		// Check if Packet is RUDP Packet
		if (!isSuccess)
		{	// Packet is not an RUDP Packet, ignore...
			Logger::PrintErrorF(__FILE__, ".... Not an RUDP Packet: expected id<%d>, actual id<%d>\n", OutPacket.RUDP_ID, OutPacket.Id());
		}
		else
		{	// RPacket successfully received. Return successful

			// DEBUG: For Debugging Purposes only
			Logger::PrintF(__FILE__, "---- Rcvd [%5lld us]: ", currentTime()); 
			Logger::PrintF(__FILE__, BasicColor::YELLOW, "ack<%d> ", OutPacket.Ack());
			Logger::PrintF(__FILE__, BasicColor::CYAN,"seq<%d> ", OutPacket.Sequence()); 
			Logger::PrintF(__FILE__, BasicColor::YELLOW, "bit<%s> ", OutPacket.AckBitfield().ToString().c_str());
			Logger::PrintF(__FILE__, "msg_id<%d> frag<%d> bytes_rcvd<%d>\n", OutPacket.MessageId(), OutPacket.FragmentCount(), static_cast<uint32_t>(OutPacket.Buffer().size()));
			return true;
		}
	}

	// Connection Timed-out
	return false;
}

bool RUDPStream::ReceiveValidPacket(RPacket& OutPacket, int32_t maxTimeOutMS)
{
	uint32_t farse;
	return ReceiveValidPacket(farse, OutPacket, maxTimeOutMS);
}

bool RUDPStream::SendPacket(uint32_t& OutBytesSent, const RPacket & packet)
{
	std::vector<uint8_t> packetData = std::move(packet.Serialize());
	int packetSize = static_cast<int>(packetData.size());

	// Send Single-Data Packet
	try
	{
		mSocket->SendTo(reinterpret_cast<char *>(packetData.data()), packetSize, mToAddress, mToPort);
	}
	catch (SocketException ex)
	{
		Logger::PrintErrorF(__FILE__, ".... sendto failed with error: %d %s\n", WSAGetLastError(), ex.what());
		return false;
	}

	// Record Sent Bytes
	OutBytesSent = (packetSize > 0) ? static_cast<uint32_t>(packetSize) : 0U;

	// DEBUG: For Debugging Purposes only
	Logger::PrintF(__FILE__, "++++ Sent [%5lld us]: seq<%d> ack<%d> bit<%s> msg_id<%d> frag<%d> bytes_sent<%d>\n"
		, currentTime(), packet.Sequence(), packet.Ack(), packet.AckBitfield().ToString().c_str()
		, packet.MessageId(), packet.FragmentCount(), static_cast<uint32_t>(packet.Buffer().size()));

	// Return Success!
	return true;
}

bool RUDPStream::SendPacket(const RPacket & packet)
{
	uint32_t farse;
	return SendPacket(farse, packet);
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
	int packetSize;
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
	currentTime(true);
	Logger::PrintF(__FILE__, "==== Begin Sending to %s:%d: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d>\n"
		, mToAddress.c_str(), mToPort, mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut);

	// Check if Fragmentation is Required
	if (sizeOfData <= sMaximumDataSize)
	{	// No Fragmentation Necessary, Simply send a single packet!

		// Create Single-Data Packet
		packet.Initialize(mSequenceNumber, mRemoteSequenceNumber, 0U, mMessageId, 1U, data, sizeOfData);
		std::vector<uint8_t> packetData = packet.Serialize();

		// Send Single-Data Packet
		packetSize = static_cast<int>(packetData.size());

		try
		{
			mSocket->SendTo(reinterpret_cast<char *>(packetData.data()), packetSize, mToAddress, mToPort);
		}
		catch (SocketException ex)
		{
			Logger::PrintErrorF(__FILE__, ".... sendto failed with error: %s\n", ex.what());
			return false;
		}
		totalBytesSent = packetSize;
		
		isSuccess = SendPacket(totalBytesSent, packet);
		assert(isSuccess);		// No Reason for this to fail...

		// Update Sequence Number
		mSequenceNumber++;

		// DEBUG: For Debug Purposes only
		//Logger::PrintF(__FILE__, "message sent [%d bytes]:\t(seq:%d ack:%d ack_bit:%d msg_size:%d)\n"
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
		ackbitfield_t localAcknowledgedBitfield(ackbitfield_t::AllTrue);	// Tracks the Received Packets from the Receiver and sets the AckBitfield accordingly
		bool isMostRecentAcknowledged = false;
		ackbitfield_t remoteAcknowledgedBitfield(ackbitfield_t::AllTrue);	// Tracks the Sent Packets Acknowledged by the Receiver
		SlidingWindow window;												// Sliding Window used to track the state of the Sent Packets
		PacketFrame currentFrame;
		PacketFrame poppedFrame;
		//std::vector<PacketFrame> packetsLost;

		// Initialize Acknowledgement characteristics
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
			packet.Initialize(mSequenceNumber, lastReceivedRemoteSequenceNumber + 1, remoteAcknowledgedBitfield, mMessageId, numberOfFragments, currentDataLocation, numOfBytesToSend);
			isSuccess = SendPacket(bytesSent, packet);
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

					Logger::PrintF(__FILE__, "==== Sending Failed. No acknowledgement received during send before escaping window.\n");

					Logger::PrintF(__FILE__, "==== Final State: [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> msg_size<%d> bytes_sent<%d>\n"
						, currentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut, sizeOfData, totalBytesSent);
					return -1;
				}
			}

			// Update Acknowledgement Bits (Shift bits same as Sliding Window)
			localAcknowledgedBitfield.bitfield <<= 1;
			localAcknowledgedBitfield.bit00 = isMostRecentAcknowledged;

			// Update Sequence Number
			++mSequenceNumber;
			isMostRecentAcknowledged = false;

			// Update bytes left to send
			currentDataLocation += numOfBytesToSend;
			currentSizeOfData -= numOfBytesToSend;
			totalBytesSent += bytesSent;

			// ====================================================================================
			// ============================ Receive Acknowledgement ===============================
			// ====================================================================================

			// Check for any Acknowledgements Received
			for (;;)
			{
				// Determine conditions if the last packet is being sent
				if (fragment >= numberOfFragments - 1)
				{	// The last packet has been sent, handle receiving of packets accordingly
					// Wait on Incoming Packets
					isSuccess = ReceiveValidPacket(packet, mMaxConnectionTimeOut);
				}
				else
				{	// More packets to be sent, Check (but do not wait) on Incoming Packets
					isSuccess = ReceiveValidPacket(packet, 0U);
				}

				if (isSuccess)
				{	// RUDP Packet Received!

					if (packet.Sequence() < mRemoteSequenceNumber)
					{	// Old Packet received...
						Logger::PrintErrorF(__FILE__, ".... Received packet for old message, ignored...\n");
						continue;
					}
					else if (packet.Sequence() >= mRemoteSequenceNumber + numberOfFragments)
					{	// Packet outside of range received...
						Logger::PrintErrorF(__FILE__, ".... Received packet for new message detected, ignored...\n");
						continue;
					}

					assert(packet.MessageId() == mMessageId);	// Should be consistent

					// =================== Handle Sequence Number =================

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

					mSequenceNumber += numberOfFragments - fragment;
					mRemoteSequenceNumber += numberOfFragments;

					Logger::PrintF(__FILE__, "==== Sending Failed. No acknowledgement received during send. Connection Timed-Out\n");

					Logger::PrintF(__FILE__, "==== Final State: [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> msg_size<%d> bytes_sent<%d>\n"
						, currentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut, sizeOfData, totalBytesSent);
					return -1;
				}

				// DEBUG: For Debugging Purposes only
				Logger::PrintF(__FILE__, BasicColor::YELLOW,"          Internal State:   local_ack_bit:  (%d) %s\n"
					, mSequenceNumber, localAcknowledgedBitfield.ToString().c_str());
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
		mRemoteSequenceNumber += numberOfFragments;
	}


	// DEBUG: For Debug Purposes Only!
	Logger::PrintF(__FILE__, "==== Successful Sending\n");
	Logger::PrintF(__FILE__, "==== Final State: [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> msg_size<%d> bytes_sent<%d>\n"
		, currentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut, sizeOfData, totalBytesSent);

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
	ackbitfield_t localAcknowledgedBitfield(ackbitfield_t::AllTrue);	// Tracks the Sent Packets Acknowledged by the Receiver
	ackbitfield_t remoteAcknowledgedBitfield(ackbitfield_t::AllTrue);	// Tracks the Received Packets from the Receiver and sets the AckBitfield accordingly
	
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

	for (;;)
	{
		// ====================================================================================
		// ================================== Receive RPacket =================================
		// ====================================================================================

		isSuccess = ReceiveValidPacket(packet, mMaxConnectionTimeOut);

		// Connection Timed-Out
		if (!isSuccess)
		{
			// TODO: Handle Connection time-outs better
			if (!isFirstPacket)
			{	// Adjust expected sequence number
				mSequenceNumber += fragmentCount;
				mRemoteSequenceNumber += fragmentCount;

				Logger::PrintF(__FILE__, "==== Receiving Failed: Connection Timed-Out ====\n");

				Logger::PrintF(__FILE__, "==== Final State [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> bytes_rcvd<0>\n"
					, currentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut);

				return -1;
			}
			else
			{
				return 0;
			}

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
			++mMessageId;

			// DEBUG: For Debugging Purposes only
			currentTime(true);
			Logger::PrintF(__FILE__, "==== Begin Receiving from %s:%d : seq<%d> remote_seq<%d> msg_id<%d> frag<%d> bytes_rcvd<%d> max_time<%d>\n"
				, mToAddress.c_str(), mToPort, mSequenceNumber, mRemoteSequenceNumber, mMessageId
				, (int)fragmentCount,packet.Buffer().size(), mMaxConnectionTimeOut);
		
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
							, currentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut);
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
			mSequenceNumber += fragmentCount;
			mRemoteSequenceNumber += fragmentCount;
			break;
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
		Logger::PrintF(__FILE__, "==== Receive Failed. Bytes received waaay too large: max<%d> rcvd<%d>\n", sizeOfBuffer, totalBytesReceived);

		Logger::PrintF(__FILE__, "==== Final State [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> bytes_rcvd<%d>\n"
			, currentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut, totalBytesReceived);
		return -1;
	}
	else if (totalBytesReceived <= 0)
	{
		Logger::PrintF(__FILE__, "==== Receive Failed. Bytes received is bad: rcvd<%d>\n", totalBytesReceived);

		Logger::PrintF(__FILE__, "==== Final State [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> bytes_rcvd<%d>\n"
			, currentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut, totalBytesReceived);

		return -1;
	}
	else
	{	// DEBUG: For Debugging Purposes only
		Logger::PrintF(__FILE__, "==== Successful Receiving\n");

		Logger::PrintF(__FILE__, "==== Final State [% 5lld us]: seq_num<%d> remote_seq_num<%d> msg_id<%d> max_time<%d> bytes_rcvd<%d>\n"
			, currentTime(), mSequenceNumber, mRemoteSequenceNumber, mMessageId, mMaxConnectionTimeOut, totalBytesReceived);
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

bool RUDPStream::Shutdown(int /*how*/)
{
	/*
	int result;

	// shutdown the sending connection
	result = shutdown(mSocket, how);
	if (result == SOCKET_ERROR)
	{
		Logger::PrintF(__FILE__, "shutdown failed with error: %d\n", WSAGetLastError());
		return false;
	}
	else 
		*/return true;
}

void RUDPStream::Close()
{

	if (mSocket.get() != nullptr)
	{
		mSocket->Close();
	}

}

bool RUDPStream::IsOpen()
{
	return  (mSocket.get() != nullptr) && (mSocket->IsOpen());
}
