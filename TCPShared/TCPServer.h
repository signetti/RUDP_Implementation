#pragma once
#include <winsock2.h>
#include <vector>
#include <string>

class TCPStream;

class TCPServer
{
private:
	// The listening socket
	SOCKET mListenSocket;
	// Address information on the listening socket
	struct addrinfo * mInfo;
	// List of clients that this server has accepted
	std::vector<TCPStream *> mClients;
	// Server port number to bind and listen to
	std::string mPort;

public:
	/**
	*	TCPServer constructor
	*	@param	port	Server port number to bind and listen to
	*/
	TCPServer(const std::string& listenPort);
	~TCPServer();

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
	TCPStream Accept();


	/**
	*	Closes any connections established by this instance
	*/
	void Close();
};

