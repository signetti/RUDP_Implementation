#pragma once
#include <winsock2.h>
#include <vector>
#include <string>
#include <chrono>

class RUDPStream;

class RUDPServer
{
private:
	struct PendingClientsT
	{
		struct sockaddr addr;
		uint32_t seqNumSent;
		uint32_t ackNumRecvd;
		std::chrono::high_resolution_clock::time_point time_stamp;
		SOCKET sock;

		PendingClientsT(const struct sockaddr& address, uint32_t sequence, uint32_t acknowledgement, std::chrono::high_resolution_clock::time_point time)
			: addr(address), seqNumSent(sequence), ackNumRecvd(acknowledgement), time_stamp(time), sock(INVALID_SOCKET) {}
	};

	std::vector<PendingClientsT> mAcknowledgeTable;
	std::vector<PendingClientsT> mAcceptedClients;

	// The listening socket
	SOCKET mListenSocket;
	// Address information on the listening socket
	struct addrinfo * mInfo;
	// List of clients that this server has accepted
	std::vector<RUDPStream *> mClients;
	// Server port number to bind and listen to
	std::string mPort;

	// How long to wait for acknowledgement of connection
	std::chrono::milliseconds mMaxConnectionTimeOut;

public:
	/**
	*	TCPServer constructor
	*	@param	port	Server port number to bind and listen to
	*/
	RUDPServer(const std::string& listenPort, std::uint32_t maxConnectionTimeOut);
	~RUDPServer();

	/**
	*	Calls CreateSocket(), Bind(), and Listen() to establish a listening socket in one function call.
	*	@return returns true if listening socket is created successfully, false otherwise.
	*/
	bool Open();

	/**
	*	Creates the socket that will be listening on the server.
	*	@return returns true if socket is created successfully, false otherwise.
	*/
	bool CreateSocket();

	/**
	*	Binds the socket created to the port number passed in on construction.
	*	@return returns true if socket binded successfully, false otherwise.
	*/
	bool Bind();


	/**
	*	Creates the socket that will be listening on the server
	*	@return returns true if successful, false otherwise.
	*/
	bool Listen();


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

