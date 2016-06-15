#pragma once
#undef min
#include "..\Include\flatbuffers\flatbuffers.h"
struct Packet;
class RPacket
{
private:
	std::uint32_t mId;
	std::uint32_t mSeq;
	std::uint32_t mAck;
	std::uint32_t mAckBitfield;
	
	std::vector<uint8_t> mBuffer;

	static std::vector<uint8_t> CopyBytes(flatbuffers::Vector<uint8_t> buffer);
	static std::vector<uint8_t> CopyBytes(uint8_t * buffer, std::uint32_t size);

	RPacket(const Packet * packet);
public:
	static const std::uint32_t RUDP_ID = 0xABCD;

	RPacket(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::vector<uint8_t> buffer);
	RPacket(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::string message);
	~RPacket();

	// Getters
	bool IsBadPacket();
	std::uint32_t Id();
	std::uint32_t Sequence();
	std::uint32_t Ack();
	std::uint32_t AckBitfield();
	std::vector<uint8_t> Buffer();
	std::string Message();
	std::vector<uint8_t> Serialize();

	// Become Bad Packet (testing)
	void BecomeBadPacket();

	static RPacket * Deserialize(std::vector<uint8_t> byte_buffer);
	static RPacket * Deserialize(uint8_t * buffer, uint32_t buffer_size);
};

