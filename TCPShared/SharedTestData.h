#pragma once
#include <string>

// Message to test on submission
static const std::string TEST_MESSAGE = "\
***************************************************************|\
===============================================================|\
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|\
---------------------------------------------------------------";
/*
// Check Sums
std::int32_t GetCheckSum(const std::string& message)
{
	int count = 0;
	std::int32_t result = 0;
	std::int32_t sum = 0;
	for (auto letter : message)
	{
		result <<= 8;
		result |= letter;

		count++;
		if (count >= 4)
		{
			sum ^= result;
			count = 0;
		}
	}
	return sum;
};
static const std::int32_t MESSAGE_CHECK_SUM = GetCheckSum(TEST_MESSAGE);

std::uint32_t RoundUpToPowerOfTwo(std::uint32_t number)
{
	--number;
	number |= number >> 1;
	number |= number >> 2;
	number |= number >> 4;
	number |= number >> 8;
	number |= number >> 16;
	++number;
	return number;

}*/

// Default Test Information
static const std::uint32_t NUM_OF_TEST_RUNS = 100;

static const std::uint32_t DEFAULT_BUFLEN = 512;//RoundUpToPowerOfTwo(TEST_MESSAGE.length() + 20);

static char * const DEFAULT_PORT = "27015";
static char * const DEFAULT_IP = "10.8.3.35";