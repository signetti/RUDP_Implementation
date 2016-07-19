#pragma once
#include <winsock2.h>
#include <map>

class WSAErrorCode
{
private:
	std::string		mCodeName;
	std::string		mDescription;
	std::string		mQuickDescription;
	std::uint32_t	mCode;

public:
	WSAErrorCode(std::uint32_t code, const std::string& codeName, const std::string& quickDescription, const std::string& description)
		: mCode(code), mCodeName(codeName), mQuickDescription(quickDescription), mDescription(description) {}
	std::string CodeName()						{ return mCodeName; }
	const std::string& CodeName() const			{ return mCodeName; }
	std::string QuickDescription()				{ return mQuickDescription; }
	const std::string& QuickDescription() const	{ return mQuickDescription; }
	std::string Description()					{ return mDescription; }
	const std::string& Description() const		{ return mDescription; }
	uint32_t Code() const						{ return mCode; }
};

class WSAManager
{
private:
#ifdef WIN32
	
	static const WSAManager _instance;
	static const std::map<uint16_t, WSAErrorCode> sWSAErrorCodeMap;
	static const WSAErrorCode WSA_NO_ERROR;

	WSADATA WinSockAppData;
#endif
	static uint16_t sWSAErrorCode;

	WSAManager();
	~WSAManager();
public:
	static void StartUp();
	static const WSAErrorCode& GetLastError();
	static void StoreLastErrorCode(int wsaError = WSAGetLastError());
	static uint16_t GetLastErrorCode();
};

