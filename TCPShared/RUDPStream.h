#pragma once
#include <winsock2.h>
#include <string>
#include <chrono>
#include <assert.h>

class RPacket;

template <typename T, uint32_t QueueSize>
class CircularQueue
{
private:
	T mQueue[QueueSize];
	std::uint32_t mEndIndex;
	std::uint32_t mSize;
	std::uint32_t mQueueSize;

public:
	static const uint32_t MaximumQueueSize = QueueSize;

	CircularQueue() : mEndIndex(0), mSize(0) {}

	// Return the object at the given index, where 0 is the oldest pushed in the queue.
	T& operator[](uint32_t index)
	{
		// Check for valid index
		if (index >= QueueSize)
		{
			throw std::exception("Index Out of Bound");
		}
		// Solve for proper index
		uint32_t properIndex = (QueueSize + (mEndIndex - mSize) + index) % QueueSize;
		// Access proper Index
		return mQueue[properIndex];
	}
	const T& operator[](uint32_t index) const { return const_cast<CircularQueue*>(this)->operator[](index); }

	T& Top(const uint32_t& offsetFromEnd = 0U) 
	{
		if (offsetFromEnd >= QueueSize)
		{
			throw std::exception("Index Out of Bound");
		}
		uint32_t lastIndex = ((QueueSize + (mEndIndex - offsetFromEnd - 1)) % QueueSize);
		return mQueue[mEndIndex - 1];
	}
	const T& Top(const uint32_t& offsetFromEnd = 0U) const { return const_cast<CircularQueue*>(this)->Top(offsetFromEnd); }

	bool Pop()
	{
		--mSize;
	}

	T PopBack()
	{
		if (mSize > 0)
		{
			--mEndIndex;
			if (mEndIndex < 0)
			{
				mEnd += QueueSize;
			}
			T popped = mQueue[mEndIndex];

			--mSize;

			return popped;
		}
	}

	bool PushBack(const T& item, T* OutPopped = nullptr)
	{
		bool result = true;
		if (mSize >= QueueSize)
		{	// Queue Overflowed, handle condition. . .
			if (OutPopped == nullptr)
			{	// Return Failed
				return false;
			}
			else
			{	// Return value overwritten in OutPopped
				uint32_t beginningIndex = (QueueSize + (mEndIndex - mSize)) % QueueSize;
				*(OutPopped) = mQueue[beginningIndex];
				--mSize;

				// Return false after Pop procedure
				result = false;
			}
		}

		// Perform Push
		mQueue[mEndIndex] = item;

		++mSize;
		++mEndIndex;
		if (mEndIndex >= QueueSize)
		{
			mEndIndex -= QueueSize;
		}

		return result;
	}

	bool IsEmpty() const { return mSize == 0; }
	bool IsFull() const { return mSize == QueueSize; }
	const std::uint32_t Size() const { return mSize; }
};




class RUDPStream
{
protected:
	/*============================= PACKET FRAME ===================================*/
	struct PacketFrame
	{
		uint32_t SequenceNumber;
		const uint8_t * DataPointer;
		uint32_t SizeOfData;
		bool IsAcknowledged;
		bool IsDataCloned;

		PacketFrame() : SequenceNumber(0), DataPointer(nullptr), SizeOfData(0), IsAcknowledged(false), IsDataCloned(false) {}
		~PacketFrame()
		{	// Free memory
			if (IsDataCloned && DataPointer != nullptr)
			{
				free(const_cast<uint8_t*>(DataPointer));
			}
		}

		// Assign Packet Frame as Bad
		void Reassign(uint32_t newSequenceNumber)
		{
			Reassign(newSequenceNumber, nullptr, 0, false);
		}

		// Assign Packet Frame, with the data signature
		void Reassign(uint32_t newSequenceNumber, const uint8_t * newDataPointer, uint32_t newSizeOfData, bool acknowledgement)
		{
			// Free memory
			if (IsDataCloned && DataPointer != nullptr)
			{
				free(const_cast<uint8_t*>(DataPointer));
			}

			SequenceNumber = newSequenceNumber;
			DataPointer = newDataPointer;
			SizeOfData = newSizeOfData;
			IsAcknowledged = acknowledgement;
			IsDataCloned = false;
		}

		// Assign Packet Frame, with the data signature (that needs to be copied over)
		void Reassign(uint32_t newSequenceNumber, const std::vector<uint8_t>& newData, bool acknowledgement)
		{
			// Free memory
			if (IsDataCloned && DataPointer != nullptr)
			{
				free(const_cast<uint8_t*>(DataPointer));
			}

			// Transfer memory over to DataPointer
			SizeOfData = newData.size();
			uint8_t * clone = reinterpret_cast<uint8_t *>(malloc(SizeOfData));
			memcpy(clone, newData.data(), static_cast<size_t>(SizeOfData));

			// Initialize Frame
			SequenceNumber = newSequenceNumber;
			DataPointer = clone;
			IsAcknowledged = acknowledgement;
			IsDataCloned = true;
		}
	};
	/*
	struct SequenceNumber
	{
	private:
		uint32_t Number;
	public:
		
		// ADDITION
		SequenceNumber& operator+=(const SequenceNumber& value) { Number += value.Number; }
		SequenceNumber& operator+=(const uint32_t& value) { Number += value; }
		friend SequenceNumber operator+(SequenceNumber lhs, const uint32_t& rhs) { lhs += rhs; return lhs; }
		friend SequenceNumber operator+(const uint32_t& lhs, SequenceNumber rhs) { rhs += lhs; return rhs; }
		friend SequenceNumber operator+(SequenceNumber lhs, const SequenceNumber& rhs) { lhs += rhs; return lhs; }
		// SUBTRACTION
		SequenceNumber& operator-=(const SequenceNumber& value) { Number -= value.Number; }
		SequenceNumber& operator-=(const uint32_t& value) { Number -= value; }
		friend SequenceNumber operator-(SequenceNumber lhs, const uint32_t& rhs) { lhs -= rhs; return lhs; }
		friend SequenceNumber operator-(const uint32_t& lhs, SequenceNumber rhs) { rhs -= lhs; return rhs; }
		friend SequenceNumber operator-(SequenceNumber lhs, const SequenceNumber& rhs) { lhs -= rhs; return lhs; }
		// Less-than Comparison
		friend SequenceNumber operator<(SequenceNumber lhs, const uint32_t& rhs) 
		{
			return lhs < rhs; return lhs;
		}
		friend SequenceNumber operator<(const uint32_t& lhs, SequenceNumber rhs) { rhs += lhs; return rhs; }
		friend SequenceNumber operator<(SequenceNumber lhs, const SequenceNumber& rhs) { lhs += rhs; return lhs; }
	};*/

	typedef CircularQueue<PacketFrame, RPacket::NumberOfAcksPerPacket> SlidingWindow;
	typedef uint32_t seq_num_t;


//protected:
public:
	// The Maximum Transmission Unit of the average router
	constexpr static const uint32_t sMaximumTransmissionUnit = 1200;
	// MTU bytes- 28 Byte IP+UDP Header - 55 byte RUDP header
	constexpr static const uint32_t sMaximumDataSize = sMaximumTransmissionUnit - 28 - 55;
	// The Buffer Size (same as the Maximum Transmission Unit)
	constexpr static const uint32_t& sBufferSize = sMaximumTransmissionUnit;
	// Sock address size for packet sending
	static const int32_t SOCK_ADDR_SIZE = sizeof(struct sockaddr_in);


	//The socket with which the stream is established
	SOCKET mSocket;
	// Address information where to send the packets
	struct sockaddr mToAddress;
	// Time (in milliseconds) to wait before determine a connection is lost
	uint32_t mMaxConnectionTimeOut;

	// The current sequence number of this sender
	seq_num_t mSequenceNumber;
	// The remote sequence number from the receiver
	seq_num_t mRemoteSequenceNumber;

	// The last message id assigned (synced between the two sides)
	seq_num_t mMessageId;

	// Last time captured since receiving any data from the remote client
	std::chrono::high_resolution_clock::time_point mLastConnectionTime;
	// Data to store in the packet
	uint8_t * mBuffer;

	/*============================= Sequence Number Wrap Around Functions ===================================*/
	constexpr static const seq_num_t MaximumSequenceNumberValue = ~(seq_num_t(0U));
	static bool IsSeq2MoreRecent(seq_num_t seq1, seq_num_t seq2)
	{
		return ((seq1 > seq2) && (seq1 - seq2 <= MaximumSequenceNumberValue / 2)) || ((seq2 > seq1) && (seq2 - seq1  > MaximumSequenceNumberValue / 2));
	}
	static int32_t DiffFromSeq1ToSeq2(seq_num_t seq1, seq_num_t seq2)
	{
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
		/* OLD WAY OF COMPUTING... QUITE ERROR-PRONE...
		int32_t diff = static_cast<int32_t>(seq2) - static_cast<int32_t>(seq1);
		if ((diff >= 0) && (diff > MaximumSequenceNumberValue / 2))
		{	// Numbers DO wrap around, compute difference
			//return diff - (MaximumSequenceNumberValue / 2);
			printf("WRAP AROUND COMPUTED: seq1<%d> seq2<%d> diff<%d>", seq1, seq2, (MaximumSequenceNumberValue - seq2) + seq1);
			return (MaximumSequenceNumberValue - seq2) + seq1;
			// Theoretically M - S2 + S1 == M + (S1 - S2) == M - diff, check on this. . .
		}
		else if((diff <= 0) && ((-diff) <= MaximumSequenceNumberValue / 2))
		{	// Numbers DO wrap around, compute difference
			//return (MaximumSequenceNumberValue / 2) - diff;
			printf("WRAP AROUND COMPUTED: seq1<%d> seq2<%d> diff<%d>", seq1, seq2, (MaximumSequenceNumberValue - seq1) + seq2);
			return (MaximumSequenceNumberValue - seq1) + seq2;
			// Theoretically M - S1 + S2 == M + (S2 - S1) == M + diff, check on this. . .
		}
		else
		{	// Numbers do not wrap around
			return diff;
		}*/
	}

	/*============================= Send / Receive Helpers ===================================*/
	bool IsConnectionTimeOut(std::chrono::high_resolution_clock::time_point startTime, uint32_t maxTimeOutMS);
	bool ReceiveRPacket(RPacket& OutPacket, uint32_t maxTimeOutMS = 10000U);
	bool SendRPacket(const RPacket& packet);
public:

	/*============================= RUDP Stream ===================================*/
	/**
	*	RUDPStream Constructor
	*	@param	socket		The socket with which the stream is established
	*/
	explicit RUDPStream(const SOCKET& socket, const struct sockaddr& toAddress
		, const uint32_t& senderSequenceNumber, const uint32_t& receiverSequenceNumber, uint32_t maxConnectionTimeOut = 2000);

	/**
	*	RUDPStream Constructor for receiving RUDP Connections only
	*	@param	socket		The socket with which the stream is established
	*/
	explicit RUDPStream(const SOCKET& socket);
	/**
	*	RUDPStream Destructor
	*/
	virtual ~RUDPStream();

	/**
	*	Copy semantics for RUDPStream
	*	@param	other	The TCPStream being copied to this instance
	*/
	RUDPStream(const RUDPStream& other);
	/**
	*	Copy semantics for RUDPStream
	*	@param	rhs		The RUDPStream being copied to this instance
	*	@return	returns a self reference
	*/
	virtual RUDPStream& operator=(const RUDPStream& rhs);

	/**
	*	Move semantics for RUDPStream
	*	@param	other	The RUDPStream being moved to this instance
	*/
	RUDPStream(RUDPStream&& other);
	/**
	*	Move semantics for RUDPStream
	*	@param	rhs		The RUDPStream being moved to this instance
	*	@return	returns a self reference
	*/
	virtual RUDPStream& operator=(RUDPStream&& rhs);

	/**
	*	Send the message to the receiving end of the stream
	*	@param	message		The message to send to the receiver
	*	@return returns the number of bytes sent. '0' means a drop in connection, and less than '0' is an error.
	*/
	int Send(const char * message);
	/**
	*	Send the message to the receiving end of the stream
	*	@param	message		The message to send to the receiver
	*	@return returns the number of bytes sent. '0' means a drop in connection, and less than '0' is an error.
	*/
	int Send(const std::string& message);
	/**
	*	Send the message to the receiving end of the stream
	*	@param	message		The message to send to the receiver, as raw bytes
	*	@return returns the number of bytes sent. '0' means a drop in connection, and less than '0' is an error.
	*/
	virtual int Send(const char * data, uint32_t sizeOfData);
	/**
	*	Send the message to the receiving end of the stream
	*	@param	message		The message to send to the receiver, as raw bytes
	*	@return returns the number of bytes sent. '0' means a drop in connection, and less than '0' is an error.
	*/
	virtual int Send(const uint8_t * data, uint32_t sizeOfData);

	/**
	*	Wait to receive a message from the end of the stream
	*	@param	OutBuffer		The buffer to write the message to
	*	@param	sizeOfBuffer	The size of the buffer
	*	@return returns the number of bytes written to the OutBuffer. '0' means a drop in connection, and less than '0' is an error.
	*/
	virtual int Receive(char * OutBuffer, uint32_t sizeOfBuffer);

	/**
	*	Shuts down a specific functionality of the stream. Simply a wrapper to the function shutdown(SOCKET,int).
	*	@param	how		How to shutdown the socket, use enumeration SHUT_RD and SHUT_WR.
	*	@return returns true if shutdown was succesful, false otherwise.
	*/
	bool Shutdown(int how);

	/**
	*	Closes the socket used in this stream.
	*/
	virtual void Close();

	/**
	*	Checks if the socket is open or valid.
	*	@return returns true if the stream is connected (open and valid), false otherwise.
	*/
	bool IsOpen();
};