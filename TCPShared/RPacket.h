#pragma once
#undef min
#include "..\Include\flatbuffers\flatbuffers.h"

struct Packet;
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
	std::uint32_t mAckBitfield;
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
	
	/*	RPacket default constructor, intended to be immediately Deserialize() data after instantiation.
	 *	@note	The id for this instance will be an invalid id (not RUDP ID) if not deserialized before use
	 */
	RPacket();
	/*	RPacket constructor
	 *	@param	seq				Sequence number
	 *	@param	ack				Acknowledgement number
	 *	@param	ack_bitfield	Acknowledgement bitfield
	 *	@param	buffer			The data this packet will hold
	 */
	RPacket(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::vector<uint8_t> buffer);
	/*	RPacket constructor
	*	@param	seq				Sequence number
	*	@param	ack				Acknowledgement number
	*	@param	ack_bitfield	Acknowledgement bitfield
	*	@param	message			The message this packet will hold
	*/
	RPacket(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::string message);
	/*	RPacket destructor
	 */
	~RPacket();

	/*	Checks to see if what is deserialized is an RUDP Packet
	*	@return	returns true if the packet is bad (id does not match RUDP ID), false otherwise
	*/
	bool IsBadPacket() const;

	// Getters (Direct manipulation is not allowed)
	std::uint32_t Id() const;
	std::uint32_t Sequence() const;
	std::uint32_t Ack() const;
	std::uint32_t AckBitfield() const;
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

