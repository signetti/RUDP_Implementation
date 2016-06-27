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
		bool bit31 : 1;
		bool bit30 : 1;
		bool bit29 : 1;
		bool bit28 : 1;
		bool bit27 : 1;
		bool bit26 : 1;
		bool bit25 : 1;
		bool bit24 : 1;
		bool bit23 : 1;
		bool bit22 : 1;
		bool bit21 : 1;
		bool bit20 : 1;
		bool bit19 : 1;
		bool bit18 : 1;
		bool bit17 : 1;
		bool bit16 : 1;
		bool bit15 : 1;
		bool bit14 : 1;
		bool bit13 : 1;
		bool bit12 : 1;
		bool bit11 : 1;
		bool bit10 : 1;
		bool bit09 : 1;
		bool bit08 : 1;
		bool bit07 : 1;
		bool bit06 : 1;
		bool bit05 : 1;
		bool bit04 : 1;
		bool bit03 : 1;
		bool bit02 : 1;
		bool bit01 : 1;
		bool bit00 : 1;
	};

	ackbitfield_t() : bitfield() {}
	ackbitfield_t(uint32_t bits) : bitfield(bits) {}
	bool operator[](uint32_t bit_number) const
	{
		assert(bit_number < 32);
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
		assert(bit_number < 32);
		switch (bit_number)
		{
		case 0:		bit00 = value;
		case 1:		bit01 = value;
		case 2:		bit02 = value;
		case 3:		bit03 = value;
		case 4:		bit04 = value;
		case 5:		bit05 = value;
		case 6:		bit06 = value;
		case 7:		bit07 = value;
		case 8:		bit08 = value;
		case 9:		bit09 = value;
		case 10:	bit10 = value;
		case 11:	bit11 = value;
		case 12:	bit12 = value;
		case 13:	bit13 = value;
		case 14:	bit14 = value;
		case 15:	bit15 = value;
		case 16:	bit16 = value;
		case 17:	bit17 = value;
		case 18:	bit18 = value;
		case 19:	bit19 = value;
		case 20:	bit20 = value;
		case 21:	bit21 = value;
		case 22:	bit22 = value;
		case 23:	bit23 = value;
		case 24:	bit24 = value;
		case 25:	bit25 = value;
		case 26:	bit26 = value;
		case 27:	bit27 = value;
		case 28:	bit28 = value;
		case 29:	bit29 = value;
		case 30:	bit30 = value;
		case 31:	bit31 = value;
		}
	}
	
	/*
	ackbitfield_t() : bitfield() {}
	ackbitfield_t(uint32_t bits) : bitfield(bits) {}
	bool operator[](uint32_t bit_number) const
	{
		assert(bit_number < 32);
		uint32_t bit_mask = (std::uint32_t(1) << bit_number);
		return (bitfield & bit_mask) != 0;
	}
	void Set(uint32_t bit_number, bool value)
	{
		assert(bit_number < 32);
		if (value)
		{
			uint32_t bit_mask = (std::uint32_t(1) << bit_number);
			bitfield |= bit_mask;
		}
		else
		{
			uint32_t bit_mask = ~(std::uint32_t(1) << bit_number);
			bitfield &= bit_mask;
		}
	}*/
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
	// The data stored in the packet, represented as an array of bytes
	std::vector<uint8_t> mBuffer;

	// Transforms the FlatBuffer's vector into the STL's vector
	static std::vector<uint8_t> CopyBytes(flatbuffers::Vector<uint8_t> buffer);
	// Transforms a pointer to an array of bytes into the STL's vector
	static std::vector<uint8_t> CopyBytes(const uint8_t * buffer, std::uint32_t size);

	// Private version of the public SerializeInstance(), with the difference of assigning an ID other than the RUDP-ID to be serialized
	static std::vector<uint8_t> SerializeInstance(std::uint32_t id, std::uint32_t seq,
		std::uint32_t ack, std::uint32_t ack_bitfield, std::vector<uint8_t> buffer);

	// Private constructor to convert Flatbuffer's Packet protocol into an RPacket instance
	RPacket(const Packet * packet);
public:
	// The RUDP's Official Identification number, used to determine if the packet is indeed an RUDP Packet
	static const std::uint32_t RUDP_ID = 0xABCD;
	
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
	RPacket(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::vector<uint8_t> buffer);
	/*	RPacket constructor
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	message			The message this packet will hold
	*/
	RPacket(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::string message);
	/*	RPacket constructor
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	data			The data this packet will hold
	*	@param	sizeOfData		The amount of bytes in the data
	*/
	RPacket(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, const uint8_t * data, std::int32_t sizeOfData);
	/*	RPacket destructor
	 */
	~RPacket();

	/*	Initialize over the RPacket (equivalent to its constructor-counterpart)
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	buffer			The data this packet will hold
	*/
	void Initialize(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::vector<uint8_t> buffer);
	/*	Initialize over the RPacket (equivalent to its constructor-counterpart)
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	message			The message this packet will hold
	*/
	void Initialize(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::string message);
	/*	Initialize over the RPacket (equivalent to its constructor-counterpart)
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	data			The data this packet will hold
	*	@param	sizeOfData		The amount of bytes in the data
	*/
	void Initialize(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, const uint8_t * data, std::int32_t sizeOfData);

	/*	Checks to see if what is deserialized is an RUDP Packet
	*	@return	returns true if the packet is bad (id does not match RUDP ID), false otherwise
	*/
	bool IsBadPacket() const;

	// Getters (Direct manipulation is not allowed)
	std::uint32_t Id() const;
	std::uint32_t Sequence() const;
	std::uint32_t Ack() const;
	ackbitfield_t AckBitfield() const;
	std::vector<uint8_t> Buffer() const;
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
	static std::vector<uint8_t> SerializeInstance(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::vector<uint8_t> buffer);

	/*	Serialize the data directly into the RUDP Packet's byte-array representation, without creating an instance.
	*	@note	Equivalent to <pre><code>RPacket(...).Serialize();</code></pre>
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	buffer			The data this packet will hold
	*	@param	bufferSize		The amount of bytes in the data
	*	@return	returns the byte-array representation of the RUDP Packet
	*/
	static std::vector<uint8_t> SerializeInstance(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, const uint8_t * buffer, uint32_t bufferSize);

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

