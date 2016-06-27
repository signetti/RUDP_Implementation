#pragma once
#include <string>

// Message to test on submission
static const std::string TEST_MESSAGE_SMALL = "\
!**************************************************************|\
===============================================================|\
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|\
--------------------------------------------------------------!\
";

static const std::string TEST_MESSAGE = 
"|Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus \
eget ex iaculis, imperdiet sem sit amet, efficitur urna. Curabitur \
rutrum lacinia justo, non maximus massa faucibus in. Sed accumsan \
ultrices purus sed tempor. Proin tempus non ante quis gravida. Duis \
elementum lacus mi, et semper leo laoreet ac. Vivamus cursus vel massa \
elementum convallis. Ut vel eros vitae erat pellentesque dictum. \
Nullam quis est sem. Integer pulvinar tempus tempus.\n\
Mauris et justo ac lectus aliquam varius in sit amet ex.Cum sociis \
natoque penatibus et magnis dis parturient montes, nascetur ridiculus \
mus.Mauris augue lacus, tincidunt et sollicitudin eget, rhoncus vel diam. \
Curabitur efficitur lacinia ex, et consequat sem faucibus placerat. Nunc \
est tellus, mattis nec risus nec, semper viverra elit.Morbi vestibulum \
rutrum iaculis.Ut eu efficitur velit, nec suscipit tellus.Nunc ultricies \
in magna a porta.Fusce ac nisl eu dui luctus commodo.Duis nec lorem \
auctor, suscipit mi non, dictum risus.Etiam molestie nulla ac velit \
rhoncus auctor.Nam tincidunt dignissim quam vulputate sodales.Class \
aptent taciti sociosqu ad litora torquent per conubia nostra, per \
inceptos himenaeos. Curabitur gravida purus non congue laoreet. Ut \
rhoncus maximus lectus, ac venenatis urna pretium non.Sed luctus \
pellentesque purus vel ornare.Mauris elit metus, lacinia vel sem vel, \
porttitor interdum ipsum.Sed sit amet ante nec ipsum ultricies \
tristique vitae quis neque.Aenean felis enim, facilisis et sem amet.|";


// Validates the test message received with the test message in the system
bool ValidateMessageReceived(const std::string& test, char * message, int size)
{
	bool isDataValid;

	// Check Validity of Data received
	isDataValid = static_cast<int>(test.length() + 1) == size;
	isDataValid = isDataValid && (test.compare(message) == 0);
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

static const std::uint32_t DEFAULT_BUFLEN = 1200;//RoundUpToPowerOfTwo(TEST_MESSAGE.length() + 20);

static uint32_t const DEFAULT_SERVER_PORT_NUMBER = 27015;
static char * const DEFAULT_SERVER_PORT = "27015";
static char * const DEFAULT_CLIENT_PORT = "27014";
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