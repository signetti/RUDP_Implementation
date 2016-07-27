#include "stdafx.h"
#include "CppUnitTest.h"

#include "RUDPSocket.h"

#pragma warning (disable:4075)
#pragma warning (disable:4307)
#pragma warning (disable:4308)

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework
		{
			template <>
			std::wstring ToString<seq_num_t>(const seq_num_t&)
			{
				return L"Hi";
			}

		}
	}
}

namespace RUDPUnitTest
{
	TEST_CLASS(SeqNumber_Test)
	{
	public:
		TEST_METHOD(PrefixPostfixIncrementDecrement_Test)
		{
			seq_num_t sequence = 0;
			Assert::AreEqual(seq_num_t(0), sequence);

			Assert::AreEqual(seq_num_t(0) , sequence++);
			Assert::AreEqual(seq_num_t(1), sequence++);
			Assert::AreEqual(seq_num_t(2), sequence++);

			Assert::AreEqual(seq_num_t(4), ++sequence);
			Assert::AreEqual(seq_num_t(5), ++sequence);
			Assert::AreEqual(seq_num_t(6), ++sequence);

			Assert::AreEqual(seq_num_t(6), sequence--);
			Assert::AreEqual(seq_num_t(5), sequence--);
			Assert::AreEqual(seq_num_t(4), sequence--);

			Assert::AreEqual(seq_num_t(2), --sequence);
			Assert::AreEqual(seq_num_t(1), --sequence);
			Assert::AreEqual(seq_num_t(0), --sequence);

			Assert::AreEqual(seq_num_t(static_cast<uint32_t>(~0)), --seq_num_t(0));
			Assert::AreEqual(seq_num_t(0), ++seq_num_t(static_cast<uint32_t>(~0)));
		}

		TEST_METHOD(AdditionSubtraction_Test)
		{
			seq_num_t sequence = 0;

			seq_num_t result;
			int32_t test_number;

			result = (seq_num_t(0) + 1);
			Assert::AreEqual(result, seq_num_t(1U));

			result = (seq_num_t(2) - 1);
			Assert::AreEqual(result, seq_num_t(1U));

			test_number = -1;
			result = (seq_num_t(2) + test_number);
			Assert::AreEqual(result, seq_num_t(1U));

			test_number = -1;
			result = (seq_num_t(25100) + test_number);
			Assert::IsTrue(result == seq_num_t(1U));
		}

		TEST_METHOD(WrapAround_Test)
		{
			seq_num_t first;
			int32_t result1, result2;

			seq_num_t differentNumbers[] = 
			{
				0,
				(uint32_t)(seq_num_t::sMaximumRangeValue / 2U),
				(uint32_t)(seq_num_t::sMinimumRangeValue / 2U),
				(uint32_t)(seq_num_t::sMaximumRangeValue),
				(uint32_t)(seq_num_t::sMinimumRangeValue),
				(uint32_t)(seq_num_t::sMaximumRangeValue + 1),
				(uint32_t)(seq_num_t::sMinimumRangeValue + 1),
			};

			for (auto second : differentNumbers)
			{
				first = second + seq_num_t::sMaximumRangeValue;
				result1 = (first - second);
				result2 = (second - first);

				{	// Test Conditional Case
					Assert::AreEqual(result1, seq_num_t::sMaximumRangeValue);
					Assert::AreEqual(result2, 0 - seq_num_t::sMaximumRangeValue);
				}
				{	// Test All Cases
					Assert::AreEqual(first == second, second == first);
					Assert::AreEqual(first != second, second != first);
					Assert::AreEqual(first > second, second < first);
					Assert::AreEqual(first >= second, second <= first);
					Assert::AreEqual(first < second, second > first);
					Assert::AreEqual(first <= second, second >= first);
				}

				first = second + seq_num_t::sMinimumRangeValue - 1;
				result1 = (first - second);
				result2 = (second - first);

				{	// Test Conditional Case
					Assert::AreEqual(result1, seq_num_t::sMaximumRangeValue);
					Assert::AreEqual(result2, 0 - seq_num_t::sMaximumRangeValue);
				}
				{	// Test All Cases
					Assert::AreEqual(first == second, second == first);
					Assert::AreEqual(first != second, second != first);
					Assert::AreEqual(first > second, second < first);
					Assert::AreEqual(first >= second, second <= first);
					Assert::AreEqual(first < second, second > first);
					Assert::AreEqual(first <= second, second >= first);
				}

				first = second + seq_num_t::sMaximumRangeValue + 1;
				result1 = (first - second);
				result2 = (second - first);

				{	// Test Very Conditional Case
					Assert::AreEqual(result1, seq_num_t::sMinimumRangeValue);
					Assert::AreEqual(result2, seq_num_t::sMinimumRangeValue);
				}
				{	// Test All Cases
					Assert::AreEqual(first == second, second == first);
					Assert::AreEqual(first != second, second != first);
					Assert::AreEqual(first > second, second < first);
					Assert::AreEqual(first >= second, second <= first);
					Assert::AreEqual(first < second, second > first);
					Assert::AreEqual(first <= second, second >= first);
				}


				first = second + seq_num_t::sMinimumRangeValue;
				result1 = (first - second);
				result2 = (second - first);

				{	// Test Very Conditional Case
					Assert::AreEqual(result1, seq_num_t::sMinimumRangeValue);
					Assert::AreEqual(result2, seq_num_t::sMinimumRangeValue);
				}
				{	// Test All Cases
					Assert::AreEqual(first == second, second == first);
					Assert::AreEqual(first != second, second != first);
					Assert::AreEqual(first > second, second < first);
					Assert::AreEqual(first >= second, second <= first);
					Assert::AreEqual(first < second, second > first);
					Assert::AreEqual(first <= second, second >= first);
				}
			}
		}
	};
}