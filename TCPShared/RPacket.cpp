#include "stdafx.h"
#include "RPacket.h"

#include "packet_generated.h"

std::vector<uint8_t> RPacket::CopyBytes(const uint8_t * buffer, const std::uint32_t size)
{
	std::uint32_t i;
	std::vector<uint8_t> vec(size);
	for (i = 0; i < size; i++)
	{
		vec[i] = buffer[i];
	}
	return vec;
}

RPacket::RPacket() : mId(0), mSeq(0), mAck(0), mAckBitfield(0), mBuffer() {}

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
	return mId != RUDP_ID;
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
	return SerializeInstance(mId, mSeq, mAck, mAckBitfield, mBuffer);
	/*
	uint8_t * packet_serialized_1;
	uint32_t size_1;

	flatbuffers::FlatBufferBuilder builder;
	
	auto buffer = builder.CreateVector(mBuffer);
	auto packet = CreatePacket(builder, mId, mSeq, mAck, mAckBitfield, static_cast<uint32_t>(mBuffer.size()), buffer);

	builder.Finish(packet);

	packet_serialized_1 = builder.GetBufferPointer();
	size_1 = builder.GetSize();
	
	return CopyBytes(packet_serialized_1, size_1);
	*/
}

void RPacket::BecomeBadPacket()
{
	int random = rand();
	mId = (random == RUDP_ID) ? random + 1 : random;
}

bool RPacket::Deserialize(std::vector<uint8_t> byte_buffer)
{
	return Deserialize(byte_buffer.data());
}

bool RPacket::Deserialize(uint8_t * buffer)
{
	// Deserialize byte buffer to packet
	const Packet * packet = GetPacket(buffer);

	// Assign Packet Values to this RPacket
	uint32_t id = packet->prot_id();
	if (id == RUDP_ID)
	{
		mId = id;
		mSeq = packet->sequence();
		mAck = packet->ack();
		mAckBitfield = packet->ack_bitfield();

		mBuffer = CopyBytes(packet->buffer()->data(), static_cast<int32_t>(packet->buffer()->size()));
		return true;
	}
	return false;
}

std::vector<uint8_t> RPacket::SerializeInstance(std::uint32_t id, std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::vector<uint8_t> buffer)
{
	uint8_t * packet_serialized;
	uint32_t packet_size;

	flatbuffers::FlatBufferBuilder builder;

	auto bufferF = builder.CreateVector(buffer);
	auto packet = CreatePacket(builder, id, seq, ack, ack_bitfield, static_cast<uint32_t>(buffer.size()), bufferF);

	builder.Finish(packet);

	packet_serialized = builder.GetBufferPointer();
	packet_size = builder.GetSize();

	return CopyBytes(packet_serialized, packet_size);
}

std::vector<uint8_t> RPacket::SerializeInstance(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::vector<uint8_t> buffer)
{
	return SerializeInstance(RUDP_ID, seq, ack, ack_bitfield, buffer);
}

RPacket * RPacket::DeserializeInstance(std::vector<uint8_t> byte_buffer)
{
	return new RPacket(GetPacket(byte_buffer.data()));
}

RPacket * RPacket::DeserializeInstance(uint8_t * buffer)
{
	return new RPacket(GetPacket(buffer));
}
