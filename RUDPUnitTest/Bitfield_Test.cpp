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
				std::uint32_t bitvalues = 0b10000111011001010100001100100001;
				//					  {_ _ _ |  _ _ _ |  _ _ _ |  _ _ _ |  _ _ _ |  _ _ _ |  _ _ _ |  _ _ _ |};
				bool boolvalues[32] = {1,0,0,0, 0,1,0,0, 1,1,0,0, 0,0,1,0, 1,0,1,0, 0,1,1,0, 1,1,1,0, 0,0,0,1};
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
		
		TEST_METHOD(Shift_Test)
		{
			ackbitfield_t Bits(true);

			Assert::IsTrue(Bits.bit31);
			Assert::IsTrue(Bits.bit00);

			Bits.bitfield <<= 1;

			Assert::IsTrue(Bits.bit31);
			Assert::IsFalse(Bits.bit00);

			Bits.bitfield >>= 2;

			Assert::IsFalse(Bits.bit31);
			Assert::IsTrue(Bits.bit00);
		}

		TEST_METHOD(Accessor_Test)
		{
			ackbitfield_t Bits(static_cast<uint32_t>(rand()));

			Assert::AreEqual(Bits[0], Bits.bit00);
			Assert::AreEqual(Bits[1], Bits.bit01);
			Assert::AreEqual(Bits[2], Bits.bit02);
			Assert::AreEqual(Bits[3], Bits.bit03);
			Assert::AreEqual(Bits[4], Bits.bit04);
			Assert::AreEqual(Bits[5], Bits.bit05);
			Assert::AreEqual(Bits[6], Bits.bit06);
			Assert::AreEqual(Bits[7], Bits.bit07);
			Assert::AreEqual(Bits[8], Bits.bit08);
			Assert::AreEqual(Bits[9], Bits.bit09);
			Assert::AreEqual(Bits[10], Bits.bit10);
			Assert::AreEqual(Bits[11], Bits.bit11);
			Assert::AreEqual(Bits[12], Bits.bit12);
			Assert::AreEqual(Bits[13], Bits.bit13);
			Assert::AreEqual(Bits[14], Bits.bit14);
			Assert::AreEqual(Bits[15], Bits.bit15);
			Assert::AreEqual(Bits[16], Bits.bit16);
			Assert::AreEqual(Bits[17], Bits.bit17);
			Assert::AreEqual(Bits[18], Bits.bit18);
			Assert::AreEqual(Bits[19], Bits.bit19);
			Assert::AreEqual(Bits[20], Bits.bit20);
			Assert::AreEqual(Bits[21], Bits.bit21);
			Assert::AreEqual(Bits[22], Bits.bit22);
			Assert::AreEqual(Bits[23], Bits.bit23);
			Assert::AreEqual(Bits[24], Bits.bit24);
			Assert::AreEqual(Bits[25], Bits.bit25);
			Assert::AreEqual(Bits[26], Bits.bit26);
			Assert::AreEqual(Bits[27], Bits.bit27);
			Assert::AreEqual(Bits[28], Bits.bit28);
			Assert::AreEqual(Bits[29], Bits.bit29);
			Assert::AreEqual(Bits[30], Bits.bit30);
			Assert::AreEqual(Bits[31], Bits.bit31);
			Assert::ExpectException<std::exception>([&]	{ Bits[32];	});
		}
	};
}