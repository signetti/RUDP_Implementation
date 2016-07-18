#include "stdafx.h"
#include "Logger.h"

#include <stdio.h>		// For variadic function
#include <stdarg.h>		// For variadic function

#include <time.h>		// For Current Date/Time

#define LOG_PRINT_SCREEN	true
#define LOG_PRINT_FILE		true

std::ofstream Logger::sLogFile;
const std::string Logger::DEFAULT_FILE_NAME = "rudp_log";
const std::string Logger::DEFAULT_FILE_EXTENSION = "txt";

Logger Logger::_instance;

#ifdef LOG_PRINT_FILE
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
#else
Logger::Logger() {}
Logger::~Logger() {}
#endif

void Logger::PrintScreen_Helper(std::vector<colored_string> messages)
{

	for (auto& message : messages)
	{
#ifdef LOG_PRINT_SCREEN
		// Print in Color
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)message.Color() | FOREGROUND_INTENSITY);
		printf("%s", message.c_str());
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#endif
#ifdef LOG_PRINT_FILE
		sLogFile << message;
#endif
	}

#ifdef LOG_PRINT_SCREEN
	printf("\n");
#endif

}

int Logger::PrintF_Helper(BasicColor color, const char * format, va_list args)
{
	int state = 0;

#ifdef LOG_PRINT_SCREEN
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color | FOREGROUND_INTENSITY);
	state = vfprintf(stdout, format, args);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#endif
#ifdef LOG_PRINT_FILE
	{
		char buffer[1024];
		memset(buffer, '\0', sizeof(buffer));
		vsnprintf(buffer, sizeof(buffer), format, args);
		sLogFile << buffer;
	}
#endif

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

void Logger::PrintScreen(const std::string & message, BasicColor color)
{
	PrintScreen_Helper({colored_string(message, color)});
}

int Logger::PrintF(const char * format, ...)
{
	// Handle Variadic Function Behavior (similar to printf)
	va_list args;
	va_start(args, format);

	// Pass down Function Call
	int state = PrintF_Helper(BasicColor::WHITE, format, args);

	// Clean Arguments
	va_end(args);

	return state;
}

int Logger::PrintF(BasicColor color, const char * format, ...)
{
	// Handle Variadic Function Behavior (similar to printf)
	va_list args;
	va_start(args, format);

	// Pass down Function Call
	int state = PrintF_Helper(color, format, args);

	// Clean Arguments
	va_end(args);

	return state;
}

void Logger::PrintError(const std::string & message)
{
	PrintScreen(message, BasicColor::RED);
}

int Logger::PrintErrorF(const char * format, ...)
{
	// Handle Variadic Function Behavior (similar to printf)
	va_list args;
	va_start(args, format);

	// Pass down Function Call
	int state = PrintF(BasicColor::RED, format, args);

	// Clean Arguments
	va_end(args);

	return state;
}

colored_string::colored_string(const char * message, BasicColor color) : std::string(message), mColor(color) {}
colored_string::colored_string(const std::string & message, BasicColor color) : std::string(message), mColor(color) {}
colored_string::colored_string(const colored_string & other) : std::string(other), mColor(other.mColor) {}
colored_string::colored_string(colored_string && other) : std::string(std::move(other)), mColor(std::move(other.mColor)) {}
