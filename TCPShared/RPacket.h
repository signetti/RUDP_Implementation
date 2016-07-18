#pragma once
#undef min
#include "..\Include\flatbuffers\flatbuffers.h"

#pragma warning (disable:4201)

struct Packet;



typedef uint32_t seq_num_t;

struct SequenceNumber
{
private:
	constexpr static const seq_num_t sMaximumSequenceNumberValue = seq_num_t(~(0U));

	uint32_t Number;
public:
	// Constructor
	SequenceNumber(uint32_t number) : Number(number) {}

	// ADDITION
	SequenceNumber& operator+=(const int32_t& value) { Number += value; return (*this); }
	friend SequenceNumber operator+(const SequenceNumber& lhs, int32_t rhs)		{ return lhs.Number + rhs; }
	friend SequenceNumber operator+(int32_t lhs, const SequenceNumber& rhs)		{ return lhs + rhs.Number; }

	// SUBTRACTION
	SequenceNumber& operator-=(const int32_t& value) { Number -= value; }
	friend SequenceNumber operator-(SequenceNumber lhs, const int32_t& rhs)		{ return lhs.Number - rhs; }
	friend SequenceNumber operator-(const int32_t& lhs, SequenceNumber rhs)		{ return lhs - rhs.Number; }

	// SUBTRACTION (Using Wrap-Around)
	friend int32_t operator-(const SequenceNumber& lhs, const SequenceNumber& rhs)
	{
		const uint32_t& seq1 = lhs.Number;
		const uint32_t& seq2 = rhs.Number;
		uint32_t diff;

		if (seq1 > seq2)
		{	// Difference will result in negative value
			diff = seq1 - seq2;
			if (diff > (sMaximumSequenceNumberValue / 2))
			{	// Negative value is actually Positive (due to wrap around)
				return static_cast<int32_t>((sMaximumSequenceNumberValue / 2) - diff);
			}
			else
			{	// Return negative difference
				return 0 - static_cast<int32_t>(diff);
			}
		}
		else
		{	// Difference will result in positive value
			diff = seq2 - seq1;
			if (diff > (sMaximumSequenceNumberValue / 2))
			{	// Positive value is actually Negative (due to wrap around)
				return 0 - static_cast<int32_t>((sMaximumSequenceNumberValue / 2) - diff);
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

	// Comparisons (Considering Wrap-Around)
	bool operator<(const SequenceNumber& rhs)	{ return ((*this) - rhs) < 0;  }
	bool operator<=(const SequenceNumber& rhs)	{ return ((*this) - rhs) <= 0; }
	bool operator>(const SequenceNumber& rhs)	{ return ((*this) - rhs) > 0;  }
	bool operator>=(const SequenceNumber& rhs)	{ return ((*this) - rhs) >= 0; }
	bool operator==(const SequenceNumber& rhs)	{ return ((*this) - rhs) == 0; }
	bool operator!=(const SequenceNumber& rhs)	{ return ((*this) - rhs) != 0; }
};

// An easy way to access the ack_bitfield
union ackbitfield_t
{
	uint32_t bitfield;
	struct
	{
		bool bit00 : 1;
		bool bit01 : 1;
		bool bit02 : 1;
		bool bit03 : 1;
		bool bit04 : 1;
		bool bit05 : 1;
		bool bit06 : 1;
		bool bit07 : 1;
		bool bit08 : 1;
		bool bit09 : 1;
		bool bit10 : 1;
		bool bit11 : 1;
		bool bit12 : 1;
		bool bit13 : 1;
		bool bit14 : 1;
		bool bit15 : 1;
		bool bit16 : 1;
		bool bit17 : 1;
		bool bit18 : 1;
		bool bit19 : 1;
		bool bit20 : 1;
		bool bit21 : 1;
		bool bit22 : 1;
		bool bit23 : 1;
		bool bit24 : 1;
		bool bit25 : 1;
		bool bit26 : 1;
		bool bit27 : 1;
		bool bit28 : 1;
		bool bit29 : 1;
		bool bit30 : 1;
		bool bit31 : 1;
	};

	constexpr static const uint32_t AllTrue	= 0b11111111111111111111111111111111;
	constexpr static const uint32_t AllFalse	= 0b00000000000000000000000000000000;

	ackbitfield_t(int32_t bits = AllFalse) : bitfield(bits) {}

	bool IsAll(bool value) const { return bitfield == ((value) ? AllTrue : AllFalse); }

	bool operator[](uint32_t bit_number) const
	{
		switch (bit_number)
		{
		case 0:		return bit00;
		case 1:		return bit01;
		case 2:		return bit02;
		case 3:		return bit03;
		case 4:		return bit04;
		case 5:		return bit05;
		case 6:		return bit06;
		case 7:		return bit07;
		case 8:		return bit08;
		case 9:		return bit09;
		case 10:	return bit10;
		case 11:	return bit11;
		case 12:	return bit12;
		case 13:	return bit13;
		case 14:	return bit14;
		case 15:	return bit15;
		case 16:	return bit16;
		case 17:	return bit17;
		case 18:	return bit18;
		case 19:	return bit19;
		case 20:	return bit20;
		case 21:	return bit21;
		case 22:	return bit22;
		case 23:	return bit23;
		case 24:	return bit24;
		case 25:	return bit25;
		case 26:	return bit26;
		case 27:	return bit27;
		case 28:	return bit28;
		case 29:	return bit29;
		case 30:	return bit30;
		case 31:	return bit31;
		default:	throw std::exception("Not a valid bit-number");
		}
	}

	void Set(uint32_t bit_number, bool value)
	{
		switch (bit_number)
		{
		case 0:		bit00 = value; break;
		case 1:		bit01 = value; break;
		case 2:		bit02 = value; break;
		case 3:		bit03 = value; break;
		case 4:		bit04 = value; break;
		case 5:		bit05 = value; break;
		case 6:		bit06 = value; break;
		case 7:		bit07 = value; break;
		case 8:		bit08 = value; break;
		case 9:		bit09 = value; break;
		case 10:	bit10 = value; break;
		case 11:	bit11 = value; break;
		case 12:	bit12 = value; break;
		case 13:	bit13 = value; break;
		case 14:	bit14 = value; break;
		case 15:	bit15 = value; break;
		case 16:	bit16 = value; break;
		case 17:	bit17 = value; break;
		case 18:	bit18 = value; break;
		case 19:	bit19 = value; break;
		case 20:	bit20 = value; break;
		case 21:	bit21 = value; break;
		case 22:	bit22 = value; break;
		case 23:	bit23 = value; break;
		case 24:	bit24 = value; break;
		case 25:	bit25 = value; break;
		case 26:	bit26 = value; break;
		case 27:	bit27 = value; break;
		case 28:	bit28 = value; break;
		case 29:	bit29 = value; break;
		case 30:	bit30 = value; break;
		case 31:	bit31 = value; break;
		default:	throw std::exception("Index Out Of Range");
		}
	}
	
	std::string ToString()
	{
		std::stringstream message;
		int i;
		for (i = 0; i < 32; ++i)
		{
			message << (this->operator[](i)) ? '1' : '0';
		}
		return message.str();
	}
};
static_assert(sizeof(ackbitfield_t) == 4, "The ackbitfield_t union is not 4 bytes");


class RPacket
{
private:
	// The Identification number that has been serialized, otherwise it will be set to the id set for RUDP
	std::uint32_t mId;
	// The sequence number of this packet, used to assert order in the data
	seq_num_t mSeq;
	// The acknowledgement number, used to alert the recipient of the last packet received
	seq_num_t mAck;
	// The acknowledgement bit-field, used to hint to the recipient of the previous 32 packets acknowledged
	ackbitfield_t mAckBitfield;
	// The message identification number, which will now be passed to start at zero
	std::uint32_t mMessageId;
	// The message identification number, which will now be passed to start at zero
	std::uint32_t mFragmentCount;
	// The data stored in the packet, represented as an array of bytes
	std::vector<uint8_t> mBuffer;

	// Transforms the FlatBuffer's vector into the STL's vector
	//static std::vector<uint8_t> CopyBytes(flatbuffers::Vector<uint8_t> buffer);
	// Transforms a pointer to an array of bytes into the STL's vector
	static std::vector<uint8_t> CopyBytes(const uint8_t * buffer, std::uint32_t size);

	// Private version of the public SerializeInstance(), with the difference of assigning an ID other than the RUDP-ID to be serialized
	static std::vector<uint8_t> SerializeInstance(std::uint32_t id, seq_num_t seq, seq_num_t ack
		, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer);

	// Private constructor to convert Flatbuffer's Packet protocol into an RPacket instance
	RPacket(const Packet * packet);



public:
	// The RUDP's Official Identification number, used to determine if the packet is indeed an RUDP Packet
	static const std::uint32_t RUDP_ID = 0xABCD;
	constexpr static const std::uint32_t NumberOfAcksPerPacket = 33U;
	
	/*	RPacket default constructor, intended to immediately Deserialize() data or Initialize() after instantiation.
	 *	@note	The id for this instance will be an invalid id (not RUDP ID) if not deserialized or initialized before use
	 */
	RPacket();
	/*	RPacket constructor
	 *	@param	seq				Sequence number
	 *	@param	ack				Acknowledgement number
	 *	@param	ack_bitfield	Acknowledgement bitfield
	 *	@param	buffer			The data this packet will hold
	 */
	RPacket(seq_num_t seq, seq_num_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer);
	/*	RPacket constructor
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	message			The message this packet will hold
	*/
	RPacket(seq_num_t seq, seq_num_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::string message);
	/*	RPacket constructor
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	data			The data this packet will hold
	*	@param	sizeOfData		The amount of bytes in the data
	*/
	RPacket(seq_num_t seq, seq_num_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, const uint8_t * data, std::int32_t sizeOfData);
	/*	RPacket destructor
	 */
	~RPacket();

	/*	Initialize over the RPacket (equivalent to its constructor-counterpart)
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	buffer			The data this packet will hold
	*/
	void Initialize(seq_num_t seq, seq_num_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer);
	/*	Initialize over the RPacket (equivalent to its constructor-counterpart)
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	message			The message this packet will hold
	*/
	void Initialize(seq_num_t seq, seq_num_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::string message);
	/*	Initialize over the RPacket (equivalent to its constructor-counterpart)
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	data			The data this packet will hold
	*	@param	sizeOfData		The amount of bytes in the data
	*/
	void Initialize(seq_num_t seq, seq_num_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, const uint8_t * data, std::int32_t sizeOfData);

	/*	Checks to see if what is deserialized is an RUDP Packet
	*	@return	returns true if the packet is bad (id does not match RUDP ID), false otherwise
	*/
	bool IsBadPacket() const;

	// Getters (Direct manipulation is not allowed)
	std::uint32_t Id() const;
	seq_num_t Sequence() const;
	seq_num_t Ack() const;
	ackbitfield_t AckBitfield() const;
	std::uint32_t MessageId() const;
	std::uint32_t FragmentCount() const;
	const std::vector<uint8_t>& Buffer() const;
	std::uint32_t BufferSize() const;
	std::string Message() const;

	//	Make this packet a Bad Packet (for testing purposes only)
	void BecomeBadPacket();

	// Serialize Data

	/*	Serialize this RUDP Packet into an array of bytes
	 *	@return	returns an array of bytes that represents the state of this instance
	 */
	std::vector<uint8_t> Serialize() const;

	/*	Deserialize the byte-array into this instance.
	 *	@param	byte_buffer			The byte-array to deserialize
	 *	@return	returns true if the buffer was successfully de-serialized into this instance and is an RUDP Packet, false otherwise
	 */
	bool Deserialize(std::vector<uint8_t> byte_buffer);

	/*	Deserialize the byte-array into this instance.
	 *	@note	Flatbuffer does not require the size of the referenced array, therefore it's not a parameter
	 *	@param	buffer			Pointer to the byte-array to deserialize
	 *	@return	returns true if the buffer was successfully de-serialized into this instance and is an RUDP Packet, false otherwise
	 */
	bool Deserialize(uint8_t * buffer);

	/*	Serialize the data directly into the RUDP Packet's byte-array representation, without creating an instance.
	 *	@note	Equivalent to <pre><code>RPacket(...).Serialize();</code></pre>
	 *	@param	seq				Sequence number
	 *	@param	ack				Acknowledgement number
	 *	@param	ack_bitfield	Acknowledgement bitfield
	 *	@param	buffer			The data this packet will hold
	 *	@return	returns the byte-array representation of the RUDP Packet
	 */
	static std::vector<uint8_t> SerializeInstance(seq_num_t seq, seq_num_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer);

	/*	Serialize the data directly into the RUDP Packet's byte-array representation, without creating an instance.
	*	@note	Equivalent to <pre><code>RPacket(...).Serialize();</code></pre>
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	buffer			The data this packet will hold
	*	@param	bufferSize		The amount of bytes in the data
	*	@return	returns the byte-array representation of the RUDP Packet
	*/
	static std::vector<uint8_t> SerializeInstance(seq_num_t seq, seq_num_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, const uint8_t * buffer, uint32_t bufferSize);

	/*	Deserialize the byte-array into a new instance.
	 *	@note	Equivalent to <pre><code>new RPacket().Deserialize(...);</code></pre>
	 *	@note	Make sure the pointer is freed properly after use (by calling <pre><code>delete</code></pre>).
	 *	@param	byte_buffer			The byte-array to deserialize
	 *	@return	returns the RPacket instance created by de-serializing
	 */
	static RPacket * DeserializeInstance(std::vector<uint8_t> byte_buffer);

	/*	Deserialize the byte-array into a new instance.
	 *	@note	Equivalent to <pre><code>new RPacket().Deserialize(...);</code></pre>
	 *	@note	Make sure the pointer is freed properly after use (by calling <pre><code>delete</code></pre>).
	 *	@param	buffer				Pointer to the byte-array to deserialize
	 *	@return	returns the RPacket instance created by de-serializing
	 */
	static RPacket * DeserializeInstance(uint8_t * buffer);
};

