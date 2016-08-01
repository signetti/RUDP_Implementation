#include "stdafx.h"


colored_string::colored_string(const char * message, BasicColor color) : std::string(message), mColor(color) {}
colored_string::colored_string(const std::string & message, BasicColor color) : std::string(message), mColor(color) {}
colored_string::colored_string(const colored_string & other) : std::string(other), mColor(other.mColor) {}
colored_string::colored_string(colored_string && other) : std::string(std::move(other)), mColor(std::move(other.mColor)) {}


std::ofstream Logger::sLogFile;
const std::string Logger::DEFAULT_FILE_NAME = "rudp_log";
const std::string Logger::DEFAULT_FILE_EXTENSION = "txt";

Logger Logger::_instance;
std::map<std::string, Logger::log_info_t> Logger::sLogInfoMap;
const Logger::log_info_t Logger::DEFAULT_LOG_INFO;

Logger::Logger()
{
	std::stringstream fileNameStream;
	fileNameStream << DEFAULT_FILE_NAME << '_' << GetTimeStamp() << '.' << DEFAULT_FILE_EXTENSION;

	std::string fileName = fileNameStream.str();
	sLogFile.open(fileName);
}

Logger::~Logger()
{
	sLogFile.close();
}

const Logger::log_info_t & Logger::GetLogInfo(const std::string & logDescriptor)
{
	auto found = sLogInfoMap.find(logDescriptor);
	if (found != sLogInfoMap.end())
	{
		return found->second;
	}
	return DEFAULT_LOG_INFO;
}

#define logInfo logInfo		// For Visual Studio coloring

void Logger::PrintScreen_Helper(const std::string& logDescriptor, const std::vector<colored_string>& messages)
{
	// Grab Logging State
	const log_info_t& logInfo = GetLogInfo(logDescriptor);
	if (logInfo.IsLoggingAllowed() == false)
	{	// Nothing to log!
		return;
	}

	if (logInfo.LogToScreen)
	{	// Print to Screen
		for (auto& message : messages)
		{
			BasicColor color = message.Color();

			// Set to Default Color if assigned
			if (color == BasicColor::DEFAULT)
			{
				color = logInfo.DefaultColor;
			}

			// Print in Color
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color | FOREGROUND_INTENSITY);
			printf("%s", message.c_str());
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
		}
		printf("\n");
	}

	if (logInfo.LogToFile)
	{	// Write to File
		for (auto& message : messages)
		{
			sLogFile << message;
		}
	}
}

int Logger::PrintF_Helper(const std::string& logDescriptor, BasicColor color, const char * format, va_list args)
{
	// Grab Logging State
	const log_info_t& logInfo = GetLogInfo(logDescriptor);
	if (logInfo.IsLoggingAllowed() == false)
	{	// Nothing to log!
		return 0;
	}

	int state = 0;

	// Set to Default Color if assigned
	if (color == BasicColor::DEFAULT)
	{
		color = logInfo.DefaultColor;
	}

	if (logInfo.LogToScreen)
	{	// Print to Screen (in Color)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color | FOREGROUND_INTENSITY);
		state = vfprintf(stdout, format, args);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	}

	if (logInfo.LogToFile)
	{	// Write to File
		char buffer[1024];
		memset(buffer, '\0', sizeof(buffer));
		vsnprintf(buffer, sizeof(buffer), format, args);
		sLogFile << buffer;
	}

	return state;
}

std::string Logger::GetTimeStamp()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
#ifdef WIN32
	localtime_s(&tstruct, &now);
#else
	tstruct = *localtime(&now);
#endif
	// Format is:				YYYY-MM-DD_HH:mm:ss
	strftime(buf, sizeof(buf), "%Y-%m-%d_%X", &tstruct);
	return buf;
}

void Logger::PrintScreen(const std::string& logDescriptor, const std::string & message, BasicColor color)
{
	PrintScreen_Helper(logDescriptor, {colored_string(message, color)});
}

int Logger::PrintF(const std::string& logDescriptor, const char * format, ...)
{
	// Handle Variadic Function Behavior (similar to printf)
	va_list args;
	va_start(args, format);

	// Pass down Function Call
	int state = PrintF_Helper(logDescriptor, BasicColor::DEFAULT, format, args);

	// Clean Arguments
	va_end(args);

	return state;
}

int Logger::PrintF(const std::string& logDescriptor, BasicColor color, const char * format, ...)
{
	// Handle Variadic Function Behavior (similar to printf)
	va_list args;
	va_start(args, format);

	// Pass down Function Call
	int state = PrintF_Helper(logDescriptor, color, format, args);

	// Clean Arguments
	va_end(args);

	return state;
}

void Logger::PrintError(const std::string& logDescriptor, const std::string & message)
{
	PrintScreen(logDescriptor, colored_string(message, BasicColor::RED));
}

int Logger::PrintErrorF(const std::string& logDescriptor, const char * format, ...)
{
	// Handle Variadic Function Behavior (similar to printf)
	va_list args;
	va_start(args, format);

	// Pass down Function Call
	int state = PrintF_Helper(logDescriptor, BasicColor::RED, format, args);

	// Clean Arguments
	va_end(args);

	return state;
}


void Logger::SetLoggerState(const std::string& logDescriptor, bool logToScreen, bool logToFile, BasicColor defaultColor)
{
	log_info_t logInfo(logToScreen, logToFile, defaultColor);
	auto& mapping = sLogInfoMap.operator[](logDescriptor);
	mapping = logInfo;
}