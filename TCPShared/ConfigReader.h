#pragma once
class ConfigReader
{
public:
	static std::vector<std::string> ReadFile(const std::string& fileName);
};
