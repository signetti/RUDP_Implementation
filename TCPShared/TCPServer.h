#pragma once
#include <winsock2.h>
#include <vector>
#include <string>

class TCPStream;

class TCPServer
{
private:
	SOCKET mListenSocket;
	struct addrinfo * mInfo;
	std::vector<TCPStream *> mClients;
	std::string mPort;

public:
	TCPServer(const std::string& listenPort);
	~TCPServer();

	bool Open();

	bool CreateSocket();

	bool Bind();

	bool Listen();

	TCPStream Accept();

	void Close();
};