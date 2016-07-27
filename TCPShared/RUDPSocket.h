#pragma once

#include "UDPSocket.h"
#include "CircularQueue.h"
#include "RPacket.h"
#include "ImmediateFunctionCall.h"

class RPacket;

/*	C++ class representation for an RUDP Socket	*/
class RUDPSocket : public UDPSocket
{
private:	/* Structures */
	struct PacketFrame
	{
		seq_num_t SequenceNumber;
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
		void Reassign(seq_num_t newSequenceNumber);

		// Assign Packet Frame, with the data signature
		void Reassign(seq_num_t newSequenceNumber, const uint8_t * newDataPointer, uint32_t newSizeOfData, bool acknowledgement);

		// Assign Packet Frame, with the data signature (that needs to be copied over)
		void Reassign(seq_num_t newSequenceNumber, const std::vector<uint8_t>& newData, bool acknowledgement);
	};

	typedef CircularQueue<PacketFrame, RPacket::NumberOfAcksPerPacket> SlidingWindow;

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
	/**	Construct a RUDP socket with no connection (must call Connect() first) binded to the given local port
	*	@param		localPort			the port this socket will bind to
	*	@param		maxTimeoutMS		the maximum time given to wait on data from the remote client (in milliseconds)
	*   @exception	SocketException		thrown if unable to create an RUDP socket
	*/
	explicit RUDPSocket(uint16_t localPort, uint32_t maxTimeoutMS = 0U);

	/**	Establishes a connection between the remote client and this client.
	*	@param	remoteAddress			the remote client's address
	*	@param	remotePort				the remote client's port
	*	@exception	SocketException		thrown if unable to establish RUDP connection
	*/
	bool Connect(const std::string& remoteAddress, uint16_t remotePort) override;

	/**	Sends the buffer to the remote client.
	*   @param	buffer					the buffer to send
	*   @param	bufferLen				the size of the buffer given
	*   @exception SocketException	thrown if unable to send data
	*/
	bool Send(const void* buffer, uint32_t bufferSize) override;


	/**	Receives a message from the remote client and passes it into the buffer.
	*   @param	buffer					the buffer for the received message to be stored
	*   @param	bufferSize				the size of the buffer given
	*   @exception SocketException		thrown if unable to send data
	*/
	uint32_t Receive(void* OutBuffer, uint32_t bufferSize) override;

private:
	// Grants access for RUDPServerSocket::Accept() connection creation
	friend class RUDPServerSocket;

	// This is not using UDP's SendTo and ReceiveFrom, so hide it
	using UDPSocket::SendTo;
	using UDPSocket::ReceiveFrom;
};


/*	RUDP server socket class	*/
class RUDPServerSocket : private UDPSocket, public IServerSocket
{
private:
	struct PendingClientsT
	{
		std::string address;
		uint16_t port;
		seq_num_t seqNumSent;
		seq_num_t ackNumRecvd;
		std::shared_ptr<RUDPSocket> socket;

		PendingClientsT(std::shared_ptr<RUDPSocket> sock, const std::string& address, uint16_t port, seq_num_t sequence, seq_num_t acknowledgement)
			: socket(sock), address(address), port(port), seqNumSent(sequence), ackNumRecvd(acknowledgement) {}
	};

private:
	std::vector<PendingClientsT> mAcknowledgeTable;
	// Available Port Number
	uint16_t mAvailablePort;

	// How long to wait for acknowledgement of connection
	std::uint32_t mMaxConnectionTimeOut;

	// List of clients accepted by this server
	std::vector<std::shared_ptr<RUDPSocket>> mClients;

public:
	/**	Construct a TCP Server Socket, listening on the given port.
	*   @param		localPort			the local port of this server socket. A value of zero will give a system-assigned unused port
	*	@param		maxTimeoutMS		the maximum time given to wait on data from a remote client (in milliseconds)
	*   @exception	SocketException		thrown if unable to create TCP server socket
	*/
	explicit RUDPServerSocket(std::uint16_t listenPort, std::uint32_t maxTimeoutMS = 0U);

	/**	Blocks until a new connection is established on this socket (or an exception is thrown)
	*   @return						a new and open RUDPSocket instance
	*   @exception	SocketException	thrown if attempt to accept a new connection fails
	*/
	RUDPSocket* Accept() override;

	/**	Retrieves the listening port number that this server is utilizing
	*	@return		the listening port number*/
	uint16_t GetListeningPort() override;
};

//LOGGER_FILE_STATE(RUDPSocket);