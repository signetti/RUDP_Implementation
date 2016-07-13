#pragma once
#undef min
#include "..\Include\flatbuffers\flatbuffers.h"

#pragma warning (disable:4201)

struct Packet;

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

	ackbitfield_t() : bitfield(0U) {}
	ackbitfield_t(bool defaultValues) : bitfield((defaultValues) ? ~(0U) : 0U) {}
	ackbitfield_t(uint32_t bits) : bitfield(bits) {}

	bool IsAll(bool value) const { return bitfield == ((value) ? ~(0U) : 0U); }

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
	std::uint32_t mSeq;
	// The acknowledgement number, used to alert the recipient of the last packet received
	std::uint32_t mAck;
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
	static std::vector<uint8_t> SerializeInstance(std::uint32_t id, std::uint32_t seq, std::uint32_t ack
		, std::uint32_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer);

	// Private constructor to convert Flatbuffer's Packet protocol into an RPacket instance
	RPacket(const Packet * packet);
public:
	// The RUDP's Official Identification number, used to determine if the packet is indeed an RUDP Packet
	static const std::uint32_t RUDP_ID = 0xABCD;
	static const std::uint32_t NumberOfAcksPerPacket = 33U;
	
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
	RPacket(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer);
	/*	RPacket constructor
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	message			The message this packet will hold
	*/
	RPacket(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::string message);
	/*	RPacket constructor
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	data			The data this packet will hold
	*	@param	sizeOfData		The amount of bytes in the data
	*/
	RPacket(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, const uint8_t * data, std::int32_t sizeOfData);
	/*	RPacket destructor
	 */
	~RPacket();

	/*	Initialize over the RPacket (equivalent to its constructor-counterpart)
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	buffer			The data this packet will hold
	*/
	void Initialize(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer);
	/*	Initialize over the RPacket (equivalent to its constructor-counterpart)
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	message			The message this packet will hold
	*/
	void Initialize(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::string message);
	/*	Initialize over the RPacket (equivalent to its constructor-counterpart)
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	data			The data this packet will hold
	*	@param	sizeOfData		The amount of bytes in the data
	*/
	void Initialize(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, const uint8_t * data, std::int32_t sizeOfData);

	/*	Checks to see if what is deserialized is an RUDP Packet
	*	@return	returns true if the packet is bad (id does not match RUDP ID), false otherwise
	*/
	bool IsBadPacket() const;

	// Getters (Direct manipulation is not allowed)
	std::uint32_t Id() const;
	std::uint32_t Sequence() const;
	std::uint32_t Ack() const;
	ackbitfield_t AckBitfield() const;
	std::uint32_t MessageId() const;
	std::uint32_t FragmentCount() const;
	const std::vector<uint8_t>& Buffer() const;
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
	static std::vector<uint8_t> SerializeInstance(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer);

	/*	Serialize the data directly into the RUDP Packet's byte-array representation, without creating an instance.
	*	@note	Equivalent to <pre><code>RPacket(...).Serialize();</code></pre>
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	buffer			The data this packet will hold
	*	@param	bufferSize		The amount of bytes in the data
	*	@return	returns the byte-array representation of the RUDP Packet
	*/
	static std::vector<uint8_t> SerializeInstance(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, const uint8_t * buffer, uint32_t bufferSize);

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

