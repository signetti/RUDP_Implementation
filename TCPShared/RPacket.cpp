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

RPacket::RPacket() : mId(0), mSeq(0), mAck(0), mAckBitfield(0U), mBuffer() {}

RPacket::RPacket(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> vec_buffer)
	: mId(RUDP_ID), mSeq(seq), mAck(ack), mAckBitfield(ack_bitfield), mMessageId(msg_id), mFragmentCount(frag_count), mBuffer(vec_buffer)
{}

RPacket::RPacket(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::string message)
	: mId(RUDP_ID), mSeq(seq), mAck(ack), mAckBitfield(ack_bitfield), mMessageId(msg_id), mFragmentCount(frag_count), mBuffer(message.begin(), message.end())
{}

RPacket::RPacket(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, const uint8_t * data, std::int32_t sizeOfData)
	: mId(RUDP_ID), mSeq(seq), mAck(ack), mAckBitfield(ack_bitfield), mMessageId(msg_id), mFragmentCount(frag_count), mBuffer(CopyBytes(data, sizeOfData))
{}

RPacket::RPacket(const Packet * packet) : mId(packet->prot_id())
	, mSeq(packet->sequence()), mAck(packet->ack()), mAckBitfield(packet->ack_bitfield())
	, mBuffer(packet->buffer()->begin(), packet->buffer()->end())
{}

RPacket::~RPacket() {}

void RPacket::Initialize(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer)
{
	mId = RUDP_ID;
	mSeq = seq;
	mAck = ack;
	mAckBitfield = ack_bitfield;
	mMessageId = msg_id;
	mFragmentCount = frag_count;
	mBuffer = buffer;
}

void RPacket::Initialize(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::string message)
{
	mId = RUDP_ID;
	mSeq = seq;
	mAck = ack;
	mAckBitfield = ack_bitfield;
	mMessageId = msg_id;
	mFragmentCount = frag_count;
	mBuffer = std::vector<uint8_t>(message.begin(), message.end());
}

void RPacket::Initialize(std::uint32_t seq, std::uint32_t ack, ackbitfield_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, const uint8_t * data, std::int32_t sizeOfData)
{
	mId = RUDP_ID;
	mSeq = seq;
	mAck = ack;
	mAckBitfield = ack_bitfield;
	mMessageId = msg_id;
	mFragmentCount = frag_count;
	mBuffer = CopyBytes(data, sizeOfData);
}

bool RPacket::IsBadPacket() const
{
	return mId != RUDP_ID;
}

std::uint32_t RPacket::Id() const
{
	return mId;
}

std::uint32_t RPacket::Sequence() const
{
	return mSeq;
}

std::uint32_t RPacket::Ack() const
{
	return mAck;
}

ackbitfield_t RPacket::AckBitfield() const
{
	return mAckBitfield;
}

std::uint32_t RPacket::MessageId() const
{
	return mMessageId;
}

std::uint32_t RPacket::FragmentCount() const
{
	return mFragmentCount;
}

const std::vector<uint8_t>& RPacket::Buffer() const
{
	return mBuffer;
}

std::string RPacket::Message() const
{
	return std::string(mBuffer.begin(), mBuffer.end());
}

std::vector<uint8_t> RPacket::Serialize() const
{
	return SerializeInstance(mId, mSeq, mAck, mAckBitfield.bitfield, mMessageId, mFragmentCount, mBuffer);
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
	uint32_t size = static_cast<uint32_t>(packet->buffer()->size());
	if (id == RUDP_ID && size < 1200)
	{
		mId = id;
		mSeq = packet->sequence();
		mAck = packet->ack();
		mAckBitfield = packet->ack_bitfield();
		mMessageId = packet->message_id();
		mFragmentCount = packet->fragment_count();

		mBuffer = CopyBytes(packet->buffer()->data(), size);
		return true;
	}
	return false;
}

std::vector<uint8_t> RPacket::SerializeInstance(std::uint32_t id, std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer)
{
	uint8_t * packet_serialized;
	uint32_t packet_size;

	flatbuffers::FlatBufferBuilder builder;

	auto bufferF = builder.CreateVector(buffer);
	auto packet = CreatePacket(builder, id, seq, ack, ack_bitfield, msg_id, frag_count, bufferF);

	builder.Finish(packet);

	packet_serialized = builder.GetBufferPointer();
	packet_size = builder.GetSize();

	return CopyBytes(packet_serialized, packet_size);
}

std::vector<uint8_t> RPacket::SerializeInstance(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, std::vector<uint8_t> buffer)
{
	return SerializeInstance(RUDP_ID, seq, ack, ack_bitfield, frag_count, msg_id, buffer);
}

std::vector<uint8_t> RPacket::SerializeInstance(std::uint32_t seq, std::uint32_t ack, std::uint32_t ack_bitfield, std::uint32_t msg_id, std::uint32_t frag_count, const uint8_t * buffer, uint32_t bufferSize)
{
	return SerializeInstance(seq, ack, ack_bitfield, msg_id, frag_count, CopyBytes(buffer, bufferSize));
}

RPacket * RPacket::DeserializeInstance(std::vector<uint8_t> byte_buffer)
{
	return new RPacket(GetPacket(byte_buffer.data()));
}

RPacket * RPacket::DeserializeInstance(uint8_t * buffer)
{
	return new RPacket(GetPacket(buffer));
}
