#pragma once
#include <string>

// Message to test on submission
static const std::string TEST_MESSAGE = "\
***************************************************************|\
===============================================================|\
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|\
---------------------------------------------------------------";

// Validates the test message received with the test message in the system
bool ValidateMessageReceived(char * message, int size)
{
	bool isDataValid;

	// Check Validity of Data received
	isDataValid = static_cast<int>(TEST_MESSAGE.length() + 1) == size;
	isDataValid = isDataValid && (TEST_MESSAGE.compare(message) == 0);
	return isDataValid;
}

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

static char * const DEFAULT_PORT = "27015";//"80";
static char * const DEFAULT_IP = "10.8.3.35";
//static char * const DEFAULT_IP = "127.0.0.1";
//static char * const DEFAULT_IP = "192.168.0.4";

//static unsigned short const RELIABLE_UDP_ID = 0xABCD;

/*
int GetError(SOCKET sock)
{
	int error = 0;
	socklen_t len = sizeof(error);
	int retval = getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len);

	if (retval != 0)
	{
		// there was a problem getting the error code
		printf("Error getting socket, error code: %d\n", WSAGetLastError());
	}
	return retval;
}*/