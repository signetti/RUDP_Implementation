#pragma once
#include <winsock2.h>
#include <vector>
#include <string>
#include <chrono>

#include <memory>

class RUDPStream;
class UDPSocket;
class SocketException;

class RUDPServer
{
private:
	struct PendingClientsT
	{
		std::string address;
		unsigned short port;
		uint32_t seqNumSent;
		uint32_t ackNumRecvd;
		std::shared_ptr<UDPSocket> socket;
		std::chrono::high_resolution_clock::time_point time_stamp;

		PendingClientsT(std::shared_ptr<UDPSocket> sock, const std::string& address,	unsigned short port, uint32_t sequence
			, uint32_t acknowledgement, std::chrono::high_resolution_clock::time_point time)
			: socket(sock), address(address), port(port), seqNumSent(sequence), ackNumRecvd(acknowledgement), time_stamp(time) {}
	};

	std::vector<PendingClientsT> mAcknowledgeTable;
	// Available Port Number
	uint16_t mAvailablePort;

	// The listening socket
	std::shared_ptr<UDPSocket> mListenSocket;
	// Address information on the listening socket
	//struct addrinfo * mInfo;
	// List of clients that this server has accepted
	std::vector<RUDPStream *> mClients;
	// Server port number to bind and listen to
	//std::string mPort;

	// How long to wait for acknowledgement of connection
	std::uint32_t mMaxConnectionTimeOut;

	static std::string IntToString(uint32_t number);

	static bool CreateNewSocket(SOCKET& sock, std::string port);
public:
	/**
	*	TCPServer constructor
	*	@param	port	Server port number to bind and listen to
	*/
	RUDPServer(std::uint16_t listenPort, std::uint32_t maxConnectionTimeOut);
	~RUDPServer();

	/**
	*	Calls CreateSocket(), Bind(), and Listen() to establish a listening socket in one function call.
	*	@return returns true if listening socket is created successfully, false otherwise.
	*/
	bool Open();


	/**
	*	Creates a TCPStream that is a connection to a client reaching this server
	*	@return returns a TCPStream connected to the client computer
	*/
	RUDPStream * Accept();


	/**
	*	Closes any connections established by this instance
	*/
	void Close();
};

