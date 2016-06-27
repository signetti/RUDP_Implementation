#include "stdafx.h"
#include "CppUnitTest.h"

//TODO: Removes warning LNK4075: ignoring "\INCREMENTAL" due to "\LTCG" specification
#include "RUDPServer.h"

#pragma warning (disable:4075)

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RUDPUnitTest
{
	TEST_CLASS(Utilities_Test)
	{
	public:

		std::string IntToString(uint32_t number)
		{
			std::stringstream message;
			message << number;
			return message.str();
		}

		TEST_METHOD(IntToString_Test)
		{
			std::string numToString[10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
			uint32_t number;
			for (number = 0; number < 10; ++number)
			{
				Assert::AreEqual(numToString[number], IntToString(number));
			}
		}
	};
}/**/