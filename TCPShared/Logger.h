#pragma once
#include <string>
#include <vector>

#include <sstream>
#include <iostream>
#include <fstream>

#include <map>

#include "ImmediateFunctionCall.h"
#include "Logger.h"

#define LOGGER_FILE_STATE_INIT(Name)		static ImmediateFunctionCall<> setLogStateFor##Name
#define LOGGER_SET_FILE_STATE_BY_NAME(Name, logToScreen, logToFile, defaultColor)\
 static ImmediateFunctionCall<> setLogStateFor##Name([] { Logger::SetLoggerState(__FILE__, logToScreen, logToFile, defaultColor); })

#define LOGGER_SET_FILE_STATE(logToScreen, logToFile, defaultColor)\
 static ImmediateFunctionCall<> setLogState([] { Logger::SetLoggerState(__FILE__, logToScreen, logToFile, defaultColor); })

/*	BasicColor class
	Enumeration of the Basic Colors that the Print screen can produce.
	Default represents the default color of the Logging system for the given request (see any Print functions)*/
enum class BasicColor
{
	BLUE	= 0x0001,	//FOREGROUND_BLUE,
	GREEN	= 0x0002,	//FOREGROUND_GREEN,
	RED		= 0x0004,	//FOREGROUND_RED,
	YELLOW	= GREEN | RED,
	CYAN	= BLUE | GREEN,
	PURPLE	= BLUE | RED,
	WHITE	= BLUE | GREEN | RED,
	DEFAULT
};

/*	Colored String.
	Takes on everything that is a string, but adds a color attribute to the string.
	This is used currently as a way to pass in string arguments with color defined for them, but PrintF can also work too.	*/
class colored_string : public std::string
{
private:
	BasicColor mColor;
public:
	colored_string(const char * message, BasicColor color = BasicColor::DEFAULT);
	colored_string(const std::string& message, BasicColor color);
	colored_string(const colored_string& other);
	colored_string(colored_string&& other);

	const BasicColor& Color() const { return mColor; }
};

/*	Logger class
*	Replaces the default printf and instead allows more flexibility on what should be done with the messages.
*	Current functionality includes:
*	 - Toggling the printing to screen and writing to file (in development) independently.
*	 - Prints to screen in a variety of colors
*	 - Keys the logging information and turns off logging based on a key
*/
class Logger
{

private:
	/*	Log Information
		The settings defined for a given log descriptor*/
	struct log_info_t
	{
		bool LogToScreen;
		bool LogToFile;
		BasicColor DefaultColor;

		log_info_t(bool logToScreen = true, bool logToFile = false, BasicColor defaultColor = BasicColor::WHITE)
			: LogToScreen(logToScreen), LogToFile(logToFile), DefaultColor(defaultColor) {}

		bool IsLoggingAllowed() const { return LogToScreen || LogToFile; }
	};

private:
	// The output file that gets logged to
	static std::ofstream sLogFile;

	// The default file name
	static const std::string DEFAULT_FILE_NAME;
	// The default file extension
	static const std::string DEFAULT_FILE_EXTENSION;

	// Logger instance used to open and close the logging file at the beginning and end of the main() function call (static construction/destruction)
	static Logger _instance;

	// Maps the log descriptor (a string in this case) to loggin information, stating what to do when a request for that descriptor is received
	static std::map<std::string, log_info_t> sLogInfoMap;
	// The default log information if the log descriptor is not found in the table (sLogInfoMap)
	static const log_info_t DEFAULT_LOG_INFO;

private:
	Logger();
	~Logger();
	
	//
	static const log_info_t& GetLogInfo(const std::string& logDescriptor);

	// Helper for the Variadic Template Function (for less code-bloat)
	static void PrintScreen_Helper(const std::string& logDescriptor, const std::vector<colored_string>& messages);

	// Helper for the Variadic Argument Function (for code re-use)
	static int PrintF_Helper(const std::string& logDescriptor, BasicColor color, const char * format, va_list args);

	// Get Time Stamp to stamp on the log files produced
	// Resource: http://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
	static std::string GetTimeStamp();


public:
	// ============ Basic Print Functions ============
	/**	Prints to screen the message with a given color 
	*	@param	logDescriptor	the key to the log info table describing how to handle the message
	*	@param	message			the message to print/log
	*	@param	color			the color to print the message (set to default color, default is defined by the log descriptor)
	*/
	static void PrintScreen(const std::string& logDescriptor, const std::string& message, BasicColor color = BasicColor::DEFAULT);

	/**	Prints to screen the messages with defined color
	*	@param	logDescriptor	the key to the log info table describing how to handle the message
	*	@param	messages		the messages to print/log and their colors
	*/
	template <class... Messages>
	static void PrintScreen(const std::string& logDescriptor, Messages... messages);

	/**	Mimics C's printf() function, with extended features
	*	@param	logDescriptor	the key to the log info table describing how to handle the message
	*	@param	format			the message to print/log in the printf() style format
	*	@return	an int, returning what printf() returns. Returns 0 if a print is not done.
	*/
	static int PrintF(const std::string& logDescriptor, const char * format, ...);

	/**	Mimics C's printf() function, with extended features
	*	@param	logDescriptor	the key to the log info table describing how to handle the message
	*	@param	color			the color to print the message (set to default color, default is defined by the log descriptor)
	*	@param	format			the message to print/log in the printf() style format
	*	@return	an int, returning what printf() returns. Returns 0 if a print is not done.
	*/
	static int PrintF(const std::string& logDescriptor, BasicColor color, const char * format, ...);

	// ============ Special Print Functions =============
	/**	printf() but with the message printed in the way errors are printed
	*	@param	logDescriptor	the key to the log info table describing how to handle the message
	*	@param	message			the messages to print/log
	*/
	static void PrintError(const std::string& logDescriptor, const std::string& message);


	/**	printf() but with the message printed in the way errors are printed
	*	@param	logDescriptor	the key to the log info table describing how to handle the message
	*	@param	format			the message to print/log in the printf() style format
	*	@return	an int, returning what printf() returns. Returns 0 if a print is not done.
	*/
	static int PrintErrorF(const std::string& logDescriptor, const char * format, ...);

	/**	Defines how this logging system will behave when given a log descriptor
	*	@param	logDescriptor	the key to the log info table describing how to handle the message
	*	@param	logToScreen		whether or not to print to the screen
	*	@param	logToFile		whether or not to write to a log file
	*	@param	defaultColor	what the default color in BasicColor::DEFAULT will be
	*/
	static void SetLoggerState(const std::string& logDescriptor, bool logToScreen, bool logToFile, BasicColor defaultColor);
};

template<class... Messages>
inline void Logger::PrintScreen(const std::string& logDescriptor, Messages... messages)
{
	std::vector<colored_string> messageList = {messages...};
	PrintScreen_Helper(logDescriptor, messageList);
}