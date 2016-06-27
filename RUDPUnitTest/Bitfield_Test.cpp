#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RUDPUnitTest
{		
	TEST_CLASS(Bitfield_Test)
	{
	public:
		
		TEST_METHOD(Initialize_Test)
		{
			// Default Initialize
			{
				ackbitfield_t Bits;
				Assert::AreEqual(Bits.bitfield, 0U);

				for (uint32_t i = 0; i < 32; ++i)
				{
					Assert::IsFalse(Bits[i]);
				}
			}

			// Initialize with 
			{
				ackbitfield_t Bits(0U);
				Assert::AreEqual(Bits.bitfield, 0U);

				for (uint32_t i = 0; i < 32; ++i)
				{
					Assert::IsFalse(Bits[i]);
				}
			}

			// Initialize with Different Values
			{
				//							...|...|...|...|...|...|...|...|
				std::uint32_t bitvalues = 0b00010010001101000101011001111000;
				//					  {_ _ _ |  _ _ _ |  _ _ _ |  _ _ _ |  _ _ _ |  _ _ _ |  _ _ _ |  _ _ _ |};
				bool boolvalues[32] = {0,0,0,1, 0,0,1,0, 0,0,1,1, 0,1,0,0, 0,1,0,1, 0,1,1,0, 0,1,1,1, 1,0,0,0};
				ackbitfield_t Bits(bitvalues);

				Assert::AreEqual(Bits.bitfield, bitvalues);


				std::wstringstream bits_message;
				for (uint32_t i = 0; i < 32; ++i)
				{
					bits_message << Bits[i];
				}

				for (uint32_t i = 0; i < 32; ++i)
				{
					std::wstringstream message;
					message << "Bit " << i << " is invalid: [" << bits_message.str() << "]";
					Assert::AreEqual(Bits[i], boolvalues[i], message.str().c_str());
				}
			}
		}

		TEST_METHOD(Set_Test)
		{
			ackbitfield_t Bits;

			// Set and Read Value
			Assert::IsFalse(Bits[0]);
			Bits.Set(0, false);
			Assert::IsFalse(Bits[0]);
			Bits.Set(0, true);
			Assert::IsTrue(Bits[0]);
			Bits.Set(0, false);
			Assert::IsFalse(Bits[0]);

			// Set and Read Random Values
			for (int32_t i = 1000; i >= 0; --i)
			{
				// Get Random Value
				uint32_t value = rand() % 32;

				// Set and Read Value
				Assert::IsFalse(Bits[value]);
				Bits.Set(value, false);
				Assert::IsFalse(Bits[value]);
				Bits.Set(value, true);
				Assert::IsTrue(Bits[value]);
				Bits.Set(value, false);
				Assert::IsFalse(Bits[value]);
			}
		}

	};
}