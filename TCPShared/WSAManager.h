#pragma once
#include <winsock2.h>
#include <map>

/* Class structure for the information on a given WSA error code */
class WSAErrorCodeInfo
{
private:
	// The name of the error given to the error code
	std::string		mCodeName;
	// The quick, one-line description for the error code
	std::string		mQuickDescription;
	// The full description for the error code
	std::string		mDescription;
	// The error code this instance represents
	std::uint32_t	mCode;

public:
	/*	Public constructor for the WSAErrorCode	*/
	WSAErrorCodeInfo(std::uint32_t code, const std::string& codeName, const std::string& quickDescription, const std::string& description)
		: mCode(code), mCodeName(codeName), mQuickDescription(quickDescription), mDescription(description) {}
	/*	Accessor to the code name
	*	@return		the code name
	*/
	const std::string& CodeName() const			{ return mCodeName; }
	/*	Accessor to the quick description
	*	@return		the quick description
	*/
	const std::string& QuickDescription() const	{ return mQuickDescription; }
	/*	Accessor to the description
	*	@return		the description
	*/
	const std::string& Description() const		{ return mDescription; }
	/*	Accessor to the error code
	*	@return		the error code
	*/
	uint32_t Code() const						{ return mCode; }
};

/*	The Singleton to handle the WSA (and all its C quirks) in a C++ fashion */
class WSAManager
{
private:
#ifdef WIN32
	// The Manager that will be called (before main) to start up the WSAManager
	static const WSAManager _instance;
	// WSA Error Code Table mapping
	static const std::map<uint16_t, WSAErrorCodeInfo> sWSAErrorCodeMap;
	// Default WSAErrorCode for when no error code info is found in the Error Code Table
	static const WSAErrorCodeInfo WSA_NO_ERROR;

	// The struct to the Winsock that is set-up
	WSADATA WinSockAppData;
#endif
	static uint16_t sWSAErrorCode;

	// Constructor to start up WSA, should only be called by the static instance
	WSAManager();
	// Destructor to start up WSA, should only be called by the static instance
	~WSAManager();

public:
	/*	Stores the Last WSA Error Code information
	*	@note	It is required to call this after any call to teh Winsock API functions, so that the last error code can be stored securely.
	*			WSAGetLastError() cannot be called outside of the stack frame the error is produced, since (for whatever reason) the function
	*			will return no error code. This is why this function is needed.
	*	@note	Use this as a parameterless function in order for the call to work.
	*/
	static void StoreLastErrorCode(int wsaError = WSAGetLastError());
	/*	Get the Last WSA Error Code (replaces WSAGetLastError() function)
	*	@return	The WSA Error Code
	*/
	static uint16_t GetLastErrorCode();
	/*	Get the Last WSA Error Code information (enhances the WSAGetLastError() function)
	*	@return	The WSA Error Code information
	*/
	static const WSAErrorCodeInfo& GetLastError();
};