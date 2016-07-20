#pragma once
#include <vector>

enum class EProtocol;

/*	Configuration Data, describing what the configuration file has stored. */
struct config_t
{
	std::string serverAddress;
	std::string message;
	uint32_t maxTimeoutMS;
	EProtocol protocol;
	bool isSendOnSuccess;
};

class ConfigReader
{
private:
	//	Get the line-by-line data from the given file name
	static std::vector<std::string> ReadFile(const std::string& fileName);
public:
	/**	Parses the file given into a struct config_t.
	*	@note	The file data should be formatted as follows:
	*			Line 1:		Server IP Address \n"\
	*			Line 2:		Protocol Type (TCP/UDP/RUDP) \n"\
	*			Line 3:		Maximum Connection Timeout (in ms) \n"\
	*			Line 4:		Is Send Complete on Success? (true/false) \n"\
	*			Line 5+:	Message to Send... \n", NUM_OF_ARGS);
	*	@param	OutResult	the output of the struct config_t, overriding the member passed in
	*	@param	fileName	the file name to read from.
	*/
	static bool ParseConfig(config_t& OutResults, const std::string& filePath);
};

