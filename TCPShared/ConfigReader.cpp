#include "stdafx.h"
#include "ConfigReader.h"

#include <iostream>
#include <fstream>
#include <string>

#include "SocketGlobal.h"
#include "Logger.h"

std::vector<std::string> ConfigReader::ReadFile(std::string const & fileName)
{
	std::vector<std::string> lines;
	std::string line;
	std::ifstream myfile(fileName);
	if (myfile.is_open())
	{
		while (std::getline(myfile, line))
		{
			if (!line.empty())
			{
				lines.push_back(line);
			}
		}
		myfile.close();
	}
	else
	{
		Logger::PrintF(__FILE__, "Config file not found\n");
	}
	return lines;
}

bool ConfigReader::ParseConfig(config_t& OutResults, const std::string& filePath)
{
	static const uint32_t NUM_OF_ARGS = 5;
	bool isFirstTime = true;

	for (;;)
	{
		if (!isFirstTime)
		{
			Logger::PrintScreen("Press \'q\' to quit, any other key to try again...");
			if (getchar() == 'q')
			{
				return false;
			}
		}
		else isFirstTime = false;

		// Read Config Files
		std::vector<std::string> configLines = ReadFile(filePath);
		if (configLines.size() < NUM_OF_ARGS)
		{
			if (!configLines.empty())
			{	// File Exists, but bad argument count
				Logger::PrintErrorF(__FILE__, "Invalid Config File Argument count. Should have at least %d lines as follows:\n"\
					"\t 1.  Server IP Address \n"\
					"\t 2.  Protocol Type (TCP/UDP/RUDP) \n"\
					"\t 3.  Maximum Connection Timeout (in ms) \n"\
					"\t 4.  Is Send Complete on Success? (true/false) \n"\
					"\t 5+. Message to Send... \n", NUM_OF_ARGS);
			}
			continue;
		}
		int lineNumber = 0;

		// Parse First Line
		OutResults.serverAddress = configLines[lineNumber];
		++lineNumber;

		// Parse Second Line
		if (_stricmp("TCP", configLines[lineNumber].c_str()) == 0)
		{
			OutResults.protocol = EProtocol::TCP;
		}
		else if (_stricmp("UDP", configLines[lineNumber].c_str()) == 0)
		{
			OutResults.protocol = EProtocol::UDP;
		}
		else if (_stricmp("RUDP", configLines[lineNumber].c_str()) == 0)
		{
			OutResults.protocol = EProtocol::RUDP;
		}
		else
		{
			Logger::PrintErrorF(__FILE__, "Error on second line: \"%s\" does not equal TCP/UDP/RUDP\n", configLines[lineNumber].c_str());
			continue;
		}
		++lineNumber;

		// Parse Third Line
		OutResults.maxTimeoutMS = atoi(configLines[lineNumber].c_str());
		if (atoi == 0U)
		{
			Logger::PrintErrorF(__FILE__, "Error on third line: \"%s\" is not a valid number\n", configLines[lineNumber].c_str());
			return false;
		}
		++lineNumber;

		// Parse Fourth Line
		OutResults.isSendOnSuccess = (_stricmp("true", configLines[lineNumber].c_str()) == 0);
		if (OutResults.isSendOnSuccess == false && _stricmp("false", configLines[lineNumber].c_str()) != 0)
		{
			Logger::PrintErrorF(__FILE__, "Error on third line: \"%s\" does not equal true/false\n", configLines[lineNumber].c_str());
			continue;
		}
		++lineNumber;

		// Parse Remaining Lines as the message

		std::stringstream message_stream;
		uint32_t i;
		for (i = lineNumber; i < configLines.size() - 1; ++i)
		{
			message_stream << configLines[i] << '\n';
		}
		message_stream << configLines[configLines.size() - 1];
		OutResults.message = message_stream.str();

		// Return success!
		return true;
	}
}