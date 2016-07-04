#include "stdafx.h"
#include "ConfigReader.h"

#include <iostream>
#include <fstream>
#include <string>


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
		printf("Config file not found\n");
	}
	return lines;
}
