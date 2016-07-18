#pragma once

#include "UDPSocket.h"
#include "CircularQueue.h"
#include "RPacket.h"

class RPacket;

/**
*   RUDP socket class
*/
class RUDPSocket : public UDPSocket
{
private:	/* Structures */
	struct PacketFrame
	{
		uint32_t SequenceNumber;
		uint8_t * DataPointer;
		uint32_t SizeOfData;
		bool IsAcknowledged;
		bool IsDataCloned;

		PacketFrame();
		~PacketFrame();

		PacketFrame(const PacketFrame& other);
		PacketFrame(PacketFrame&& other);

		PacketFrame& operator=(const PacketFrame& rhs);
		PacketFrame& operator=(PacketFrame&& rhs);

		// Assign Packet Frame as Bad
		void Reassign(uint32_t newSequenceNumber);

		// Assign Packet Frame, with the data signature
		void Reassign(uint32_t newSequenceNumber, const uint8_t * newDataPointer, uint32_t newSizeOfData, bool acknowledgement);

		// Assign Packet Frame, with the data signature (that needs to be copied over)
		void Reassign(uint32_t newSequenceNumber, const std::vector<uint8_t>& newData, bool acknowledgement);
	};

	typedef CircularQueue<PacketFrame, RPacket::NumberOfAcksPerPacket> SlidingWindow;

	friend class RUDPServerSocket;

private:	/* Variables */

	// The Maximum Transmission Unit of the average router
	constexpr static const uint32_t sMaximumTransmissionUnit = 1200;
	// MTU bytes- 28 Byte IP+UDP Header - 55 byte RUDP header
	constexpr static const uint32_t sMaximumDataSize = sMaximumTransmissionUnit - 28 - 55;
	// The Buffer Size (same as the Maximum Transmission Unit)
	constexpr static const uint32_t& sBufferSize = sMaximumTransmissionUnit;
	// Sock address size for packet sending
	static const int32_t SOCK_ADDR_SIZE = sizeof(struct sockaddr_in);


	// The current sequence number of this sender
	seq_num_t mSequenceNumber;
	// The remote sequence number from the receiver
	seq_num_t mRemoteSequenceNumber;

	// The last message id assigned (synced between the two sides)
	uint32_t mMessageId;

private:	/* Functions */
	bool ReceiveValidPacket(uint8_t* buffer, uint32_t bufferSize, seq_num_t minSeq, uint32_t fragmentCount, RPacket& OutPacket);
	bool ReceiveValidPacket(uint8_t* buffer, uint32_t bufferSize, seq_num_t minSeq, uint32_t fragmentCount, RPacket& OutPacket, uint32_t maxTimeout);
	bool SendPacket(const RPacket& packet);

	void MoveToNextMessage(uint32_t fragmentCount);

public:

	/**
	*   Construct a UDP socket with the given local port
	*   @param localPort local port
	*   @exception SocketException thrown if unable to create UDP socket
	*/
	explicit RUDPSocket(uint16_t localPort, uint32_t maxTimeoutMS = 0U);

	bool Connect(const std::string& remoteAddress, uint16_t remotePort) override;

	/**
	*   Send the given buffer as a UDP datagram to the
	*   specified address/port
	*   @param buffer buffer to be written
	*   @param bufferLen number of bytes to write
	*   @param foreignAddress address (IP address or name) to send to
	*   @param foreignPort port number to send to
	*   @return true if send is successful
	*   @exception SocketException thrown if unable to send datagram
	*/
	bool Send(const void* buffer, uint32_t bufferSize) override;

	/**
	*   Read read up to bufferLen bytes data from this socket.  The given buffer
	*   is where the data will be placed
	*   @param buffer buffer to receive data
	*   @param bufferLen maximum number of bytes to receive
	*   @param sourceAddress address of datagram source
	*   @param sourcePort port of data source
	*   @return number of bytes received and -1 for error
	*   @exception SocketException thrown if unable to receive datagram
	*/
	uint32_t Receive(void* OutBuffer, uint32_t bufferSize) override;

private:
	using UDPSocket::SendTo;
	using UDPSocket::ReceiveFrom;
	//bool SendTo(const void* buffer, uint32_t bufferSize, const std::string& remoteAddress, uint16_t remotePort) override;
	//bool ReceiveFrom(void* OutBuffer, uint32_t& InOutBufferSize, std::string& OutRemoteAddress, uint16_t& OutRemotePort, uint32_t maxTimeoutMS = 0U) override;
};

class RUDPServerSocket : private UDPSocket, public IServerSocket
{
private:
	struct PendingClientsT
	{
		std::string address;
		uint16_t port;
		uint32_t seqNumSent;
		uint32_t ackNumRecvd;
		std::shared_ptr<RUDPSocket> socket;

		PendingClientsT(std::shared_ptr<RUDPSocket> sock, const std::string& address, uint16_t port, uint32_t sequence, uint32_t acknowledgement)
			: socket(sock), address(address), port(port), seqNumSent(sequence), ackNumRecvd(acknowledgement)
		{}
	};

private:
	std::vector<PendingClientsT> mAcknowledgeTable;
	// Available Port Number
	uint16_t mAvailablePort;

	// How long to wait for acknowledgement of connection
	std::uint32_t mMaxConnectionTimeOut;

	// List of clients that this server has accepted
	std::vector<std::shared_ptr<RUDPSocket>> mClients;

public:
	explicit RUDPServerSocket(std::uint16_t listenPort, std::uint32_t maxConnectionTimeOut = 0U);

	RUDPSocket* Accept() override;
	uint16_t GetListeningPort() override;
};