#pragma once
#include <string>
#include <vector>

#include <sstream>
#include <iostream>
#include <fstream>

#include "ImmediateFunctionCall.h"
/*	Macro Helpers
#define FOREGROUND_CYAN	FOREGROUND_BLUE | FOREGROUND_GREEN
#define FOREGROUND_YELLOW	FOREGROUND_GREEN |FOREGROUND_RED

#define Logger::PrintF_COLOR(Color,...)																						\
{																													\
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color | FOREGROUND_INTENSITY);							\
	Logger::PrintF(__VA_ARGS__);																							\
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)	\
}

#define Logger::PrintF_ERROR(...)	Logger::PrintF_COLOR(FOREGROUND_RED, __VA_ARGS__);

*/
/*	Basic Colors
*/
enum class BasicColor
{
	BLUE	= FOREGROUND_BLUE,
	GREEN	= FOREGROUND_GREEN,
	RED		= FOREGROUND_RED,
	YELLOW	= GREEN | RED,
	CYAN	= BLUE | GREEN,
	PURPLE	= BLUE | RED,
	WHITE	= BLUE | GREEN | RED,
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
	static std::ofstream sLogFile;
	static const std::string DEFAULT_FILE_NAME;
	static const std::string DEFAULT_FILE_EXTENSION;

	static Logger _instance;
private:
	Logger();
	~Logger();
	
	// Helper for the Variadic Template Function (for less code-bloat)
	static void PrintScreen_Helper(std::vector<colored_string> messages);

	// Helper for the Variadic Template Function (for less code-bloat)
	static int PrintF_Helper(BasicColor color, const char * format, va_list args);

	// Get Time Stamp
	// http://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
	static std::string GetTimeStamp();
public:
	// ============ Basic Print Functions ============
	static void PrintScreen(const std::string& message, BasicColor color = BasicColor::WHITE);

	template <class... Messages>
	static void PrintScreen(Messages... messages);

	static int PrintF(const char * formatted, ...);

	static int PrintF(BasicColor color, const char * formatted, ...);

	// ============ Special Print Functions =============
	static void PrintError(const std::string& message);

	static int PrintErrorF(const char * format, ...);
};

template<class... Messages>
inline void Logger::PrintScreen(Messages... messages)
{
	std::vector<colored_string> messageList;
	for (colored_string& message : messages)
	{
		messageList.push_back(message);
	}

	PrintScreen_Helper(messageList);
}
