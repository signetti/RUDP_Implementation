#pragma once
#include <string>
#include <vector>

#include <sstream>
#include <iostream>
#include <fstream>

#include <map>

#include "ImmediateFunctionCall.h"

#define LOGGER_SET_FILE_STATE(LogToScreen, LogToFile, DefaultColor)\
 static ImmediateFunctionCall<> setLogState([] { Logger::SetLoggerState(__FILE__, LogToScreen, LogToFile, DefaultColor); })

/*	Basic Colors
*/
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

/* Colored String
*/
class colored_string : public std::string
{
private:
	BasicColor mColor;
public:
	colored_string(const char * message, BasicColor color = BasicColor::WHITE);
	colored_string(const std::string& message, BasicColor color);
	colored_string(const colored_string& other);
	colored_string(colored_string&& other);

	const BasicColor& Color() const { return mColor; }
};

/*	Logger
*/
class Logger
{

private:
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
	static std::ofstream sLogFile;
	static const std::string DEFAULT_FILE_NAME;
	static const std::string DEFAULT_FILE_EXTENSION;

	static Logger _instance;

	static std::map<std::string, log_info_t> sLogInfoMap;
	static const log_info_t DEFAULT_LOG_INFO;

private:
	Logger();
	~Logger();
	
	//
	static const log_info_t& GetLogInfo(const std::string& logDescriptor);

	// Helper for the Variadic Template Function (for less code-bloat)
	static void PrintScreen_Helper(const std::string& logDescriptor, const std::vector<colored_string>& messages);

	// Helper for the Variadic Template Function (for less code-bloat)
	static int PrintF_Helper(const std::string& logDescriptor, BasicColor color, const char * format, va_list args);

	// Get Time Stamp
	// http://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
	static std::string GetTimeStamp();


public:
	// ============ Basic Print Functions ============
	static void PrintScreen(const std::string& logDescriptor, const std::string& message, BasicColor color = BasicColor::DEFAULT);

	template <class... Messages>
	static void PrintScreen(const std::string& logDescriptor, Messages... messages);

	static int PrintF(const std::string& logDescriptor, const char * formatted, ...);

	static int PrintF(const std::string& logDescriptor, BasicColor color, const char * formatted, ...);

	// ============ Special Print Functions =============
	static void PrintError(const std::string& logDescriptor, const std::string& message);

	static int PrintErrorF(const std::string& logDescriptor, const char * format, ...);

	static void SetLoggerState(const std::string& logDescriptor, bool LogToScreen, bool LogToFile, BasicColor DefaultColor)
	{
		log_info_t logInfo(LogToScreen, LogToFile, DefaultColor);
		auto& mapping = sLogInfoMap.operator[](logDescriptor);
		mapping = logInfo;
	}
};

template<class... Messages>
inline void Logger::PrintScreen(const std::string& logDescriptor, Messages... messages)
{
	std::vector<colored_string> messageList = {messages...};
	PrintScreen_Helper(logDescriptor, messageList);
}