#include "stdafx.h"
#include "RPacket.h"

#include "packet_generated.h"

std::vector<uint8_t> RPacket::CopyBytes(uint8_t * buffer, const std::uint32_t size)
{
	std::uint32_t i;
	std::vector<uint8_t> vec(size);
	for (i = 0; i < size; i++)
	{
		vec[i] = buffer[i];
	}
	return vec;
}

RPacket::RPacket(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::vector<uint8_t> vec_buffer)
	: mId(RUDP_ID), mSeq(seq), mAck(ack), mAckBitfield(ack_bitfield), mBuffer(vec_buffer)
{}

RPacket::RPacket(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::string message)
	: mId(RUDP_ID), mSeq(seq), mAck(ack), mAckBitfield(ack_bitfield), mBuffer(message.begin(), message.end())
{}

RPacket::RPacket(const Packet * packet) : mId(packet->prot_id())
	, mSeq(packet->sequence()), mAck(packet->ack()), mAckBitfield(packet->ack_bitfield())
	, mBuffer(packet->buffer()->begin(), packet->buffer()->end())
{}

RPacket::~RPacket() {}

bool RPacket::IsBadPacket() 
{
	int ID = mId;
	int NID = RUDP_ID;
	return ID != NID;
	//return mId != RUDP_ID;
}

std::uint32_t RPacket::Id()
{
	return mId;
}

std::uint32_t RPacket::Sequence()
{
	return mSeq;
}

std::uint32_t RPacket::Ack()
{
	return mAck;
}

std::uint32_t RPacket::AckBitfield()
{
	return mAckBitfield;
}

std::vector<uint8_t> RPacket::Buffer()
{
	return mBuffer;
}

std::string RPacket::Message()
{
	return std::string(mBuffer.begin(), mBuffer.end());
}

std::vector<uint8_t> RPacket::Serialize()
{
	uint8_t * packet_serialized_1;
	uint32_t size_1;

	flatbuffers::FlatBufferBuilder builder;
	
	auto buffer = builder.CreateVector(mBuffer);
	auto packet = CreatePacket(builder, mId, mSeq, mAck, mAckBitfield, static_cast<uint32_t>(mBuffer.size()), buffer);

	builder.Finish(packet);

	packet_serialized_1 = builder.GetBufferPointer();
	size_1 = builder.GetSize();
	
	return CopyBytes(packet_serialized_1, size_1);
}

void RPacket::BecomeBadPacket()
{
	int random = rand();
	mId = (random == RUDP_ID) ? random + 1 : random;
}

RPacket * RPacket::Deserialize(std::vector<uint8_t> byte_buffer)
{
	return new RPacket(GetPacket(byte_buffer.data()));
}

RPacket * RPacket::Deserialize(uint8_t * buffer, uint32_t)
{
	return new RPacket(GetPacket(buffer));
}
