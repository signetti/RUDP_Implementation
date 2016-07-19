#pragma once
#include <string>
#include "ConfigReader.h"
#include "Logger.h"

// Message to test on submission
static const std::string TEST_MESSAGE_SMALL = "\
!**************************************************************|\
===============================================================|\
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|\
--------------------------------------------------------------!\
";

/*
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
*/

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
tristique vitae quis neque.Aenean felis enim, facilisis et sem amet.\
Quisque vehicula commodo ullamcorper.Morbi non consectetur urna. \
Crassuscipit eu arcu sed volutpat.Aenean lectus velit, fringilla vitae \
purus non, commodo laoreet turpis.Cum sociis natoque penatibus et magnis \
dis parturient montes, nascetur ridiculus mus.Nullam posuere augue et \
justo condimentum, ut mollis elit rhoncus.Nunc convallis nibh vestibulum \
nulla porttitor, eget laoreet sapien mollis.Praesent ornare, augue sit \
amet tristique placerat, orci eros volutpat mi, a convallis dui metus id \
lacus.Phasellus sed eros eget enim consequat dapibus.Integer viverra \
condimentum euismod.Pellentesque habitant morbi tristique senectus et \
netus et malesuada fames ac turpis egestas. \n\
Mauris et justo ac lectus aliquam varius in sit amet ex.Cum sociis \
natoque penatibus et magnis dis parturient montes, nascetur ridiculus \
mus.Mauris augue lacus, tincidunt et sollicitudin eget, rhoncus vel diam. \
Curabitur efficitur lacinia ex, et consequat sem faucibus placerat.Nunc \
est tellus, mattis nec risus nec, semper viverra elit.Morbi vestibulum \
rutrum iaculis.Ut eu efficitur velit, nec suscipit tellus.Nunc ultricies \
in magna a porta.Fusce ac nisl eu dui luctus commodo.Duis nec lorem \
auctor, suscipit mi non, dictum risus.Etiam molestie nulla ac velit \
rhoncus auctor.Nam tincidunt dignissim quam vulputate sodales.Class \
aptent taciti sociosqu ad litora torquent per conubia nostra, per \
inceptos himenaeos.Curabitur gravida purus non congue laoreet.Ut \
rhoncus maximus lectus, ac venenatis urna pretium non.Sed luctus \
pellentesque purus vel ornare.Mauris elit metus, lacinia vel sem vel, \
porttitor interdum ipsum.Sed sit amet ante nec ipsum ultricies \
eget ex iaculis, imperdiet sem sit amet, efficitur urna.Curabitur \
rutrum lacinia justo, non maximus massa faucibus in.Sed accumsan \
ultrices purus sed tempor.Proin tempus non ante quis gravida.Duis \
elementum lacus mi, et semper leo laoreet ac.Vivamus cursus vel massa \
elementum convallis.Ut vel eros vitae erat pellentesque dictum. \
Nullam quis est sem.Integer pulvinar tempus tempus.\n\
Mauris et justo ac lectus aliquam varius in sit amet ex.Cum sociis \
natoque penatibus et magnis dis parturient montes, nascetur ridiculus \
mus.Mauris augue lacus, tincidunt et sollicitudin eget, rhoncus vel diam. \
Curabitur efficitur lacinia ex, et consequat sem faucibus placerat.Nunc \
est tellus, mattis nec risus nec, semper viverra elit.Morbi vestibulum \
rutrum iaculis.Ut eu efficitur velit, nec suscipit tellus.Nunc ultricies \
in magna a porta.Fusce ac nisl eu dui luctus commodo.Duis nec lorem \
auctor, suscipit mi non, dictum risus.Etiam molestie nulla ac velit \
rhoncus auctor.Nam tincidunt dignissim quam vulputate sodales.Class \
aptent taciti sociosqu ad litora torquent per conubia nostra, per \
inceptos himenaeos.Curabitur gravida purus non congue laoreet.Ut \
rhoncus maximus lectus, ac venenatis urna pretium non.Sed luctus \
pellentesque purus vel ornare.Mauris elit metus, lacinia vel sem vel, \
porttitor interdum ipsum.Sed sit amet ante nec ipsum ultricies \
eget ex iaculis, imperdiet sem sit amet, efficitur urna.Curabitur \
rutrum lacinia justo, non maximus massa faucibus in.Sed accumsan \
ultrices purus sed tempor.Proin tempus non ante quis gravida.Duis \
elementum lacus mi, et semper leo laoreet ac.Vivamus cursus vel massa \
elementum convallis.Ut vel eros vitae erat pellentesque dictum. \
Nullam quis est sem.Integer pulvinar tempus tempus.\n\
Mauris et justo ac lectus aliquam varius in sit amet ex.Cum sociis \
natoque penatibus et magnis dis parturient montes, nascetur ridiculus \
mus.Mauris augue lacus, tincidunt et sollicitudin eget, rhoncus vel diam. \
Curabitur efficitur lacinia ex, et consequat sem faucibus placerat.Nunc \
est tellus, mattis nec risus nec, semper viverra elit.Morbi vestibulum \
rutrum iaculis.Ut eu efficitur velit, nec suscipit tellus.Nunc ultricies \
in magna a porta.Fusce ac nisl eu dui luctus commodo.Duis nec lorem \
auctor, suscipit mi non, dictum risus.Etiam molestie nulla ac velit \
rhoncus auctor.Nam tincidunt dignissim quam vulputate sodales.Class \
aptent taciti sociosqu ad litora torquent per conubia nostra, per \
inceptos himenaeos.Curabitur gravida purus non congue laoreet.Ut \
rhoncus maximus lectus, ac venenatis urna pretium non.Sed luctus \
pellentesque purus vel ornare.Mauris elit metus, lacinia vel sem vel, \
porttitor interdum ipsum.Sed sit amet ante nec ipsum ultricies \
tristique vitae quis neque.Aenean felis enim, facilisis et sem amet.\
Quisque vehicula commodo ullamcorper.Morbi non consectetur urna. \
Crassuscipit eu arcu sed volutpat.Aenean lectus velit, fringilla vitae \
purus non, commodo laoreet turpis.Cum sociis natoque penatibus et magnis \
dis parturient montes, nascetur ridiculus mus.Nullam posuere augue et \
justo condimentum, ut mollis elit rhoncus.Nunc convallis nibh vestibulum \
nulla porttitor, eget laoreet sapien mollis.Praesent ornare, augue sit \
amet tristique placerat, orci eros volutpat mi, a convallis dui metus id \
lacus.Phasellus sed eros eget enim consequat dapibus.Integer viverra \
condimentum euismod.Pellentesque habitant morbi tristique senectus et \
netus et malesuada fames ac turpis egestas. \
Morbi mattis magna non nulla accumsan, at dictum dui venenatis. Integer \
ut sodales justo.Aliquam erat volutpat.Vivamus efficitur nibh est, in \
faucibus ligula interdum eu.Aliquam vel venenatis justo, vitae fringilla \
enim.Nunc cursus dictum nulla non molestie.Etiam sit amet mattis lorem, \
id sodales ex.Sed at tellus porttitor volutpat.|";

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

static const std::uint32_t DEFAULT_BUFLEN = 1200;

static uint16_t const DEFAULT_SERVER_PORT_NUMBER = 27015;
static uint16_t const DEFAULT_CLIENT_PORT_NUMBER = 28015;

static char * const CONFIG_FILE_PATH = "../Content/config.txt";

enum class EProtocol
{
	TCP,
	UDP,
	RUDP
};

struct config_t
{
	std::string serverAddress;
	std::string message;
	uint32_t maxTimeoutMS;
	EProtocol protocol;
	bool isSendOnSuccess;
};

bool ParseConfig(config_t& OutResults)
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
		std::vector<std::string> configLines = ConfigReader::ReadFile(CONFIG_FILE_PATH);
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
			Logger::PrintErrorF(__FILE__, "Error on second line: \"%s\" does not equal TCP/UDP/RUDP\n", configLines[1].c_str());
			continue;
		}
		++lineNumber;

		// Parse Third Line
		OutResults.maxTimeoutMS = atoi(configLines[lineNumber].c_str());
		if (atoi == 0U)
		{
			Logger::PrintErrorF(__FILE__, "Error on third line: \"%s\" is not a valid number\n", configLines[1].c_str());
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