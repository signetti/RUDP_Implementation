#include "stdafx.h"
#include "CppUnitTest.h"

#include "RUDPSocket.h"

#pragma warning (disable:4075)

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RUDPUnitTest
{
	TEST_CLASS(CircularQueue_Test)
	{
	public:

		TEST_METHOD(Queue_Test)
		{
			CircularQueue<std::string, 4> queue;

			
			// ========= Initial State ========

			// Test States
			Assert::IsTrue(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 0U);

			bool hasSizeChanged;
			hasSizeChanged = queue.Pop();
			Assert::IsFalse(hasSizeChanged);

			Assert::ExpectException<std::exception>([&] { queue.PopBack(); });
			Assert::ExpectException<std::exception>([&] { queue.Back(); });
			Assert::ExpectException<std::exception>([&] { queue[0]; });

			// ========== One Element ==========

			// Push One
			std::string message("Am I popped?");
			std::string popped(message);

			std::string items[] = {"Hello", "What's Up?", "Not Much", "How about you?", "I'm fine too", "Goodbye", "World"};
			
			hasSizeChanged = queue.PushBack(items[0], &popped);
			Assert::IsTrue(hasSizeChanged);
			Assert::AreEqual(message, popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 1U);

			Assert::AreEqual(queue.Back(), items[0]);
			Assert::ExpectException<std::exception>([&] { queue.Back(1); });
			Assert::AreEqual(queue[0], items[0]);
			Assert::ExpectException<std::exception>([&] { queue[1]; });

			// Pop One
			hasSizeChanged = queue.Pop();
			Assert::IsTrue(hasSizeChanged);

			// Test State
			Assert::IsTrue(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 0U);

			hasSizeChanged = queue.Pop();
			Assert::IsFalse(hasSizeChanged);

			Assert::ExpectException<std::exception>([&] { queue.PopBack(); });
			Assert::ExpectException<std::exception>([&] { queue.Back(); });
			Assert::ExpectException<std::exception>([&] { queue[0]; });

			// ====== Multiple Elements =====

			// Push One Again
			hasSizeChanged = queue.PushBack(items[0], &popped);
			Assert::IsTrue(hasSizeChanged);
			Assert::AreEqual(message, popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 1U);

			Assert::AreEqual(queue.Back(), items[0]);
			Assert::ExpectException<std::exception>([&] { queue.Back(1); });
			Assert::AreEqual(queue[0], items[0]);
			Assert::ExpectException<std::exception>([&] { queue[1]; });

			// Push Second
			hasSizeChanged = queue.PushBack(items[1], &popped);
			Assert::IsTrue(hasSizeChanged);
			Assert::AreEqual(message, popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 2U);

			Assert::AreEqual(queue.Back(), items[0]);
			Assert::AreEqual(queue.Back(1), items[1]);
			Assert::ExpectException<std::exception>([&] { queue.Back(2); });
			Assert::AreEqual(queue[0], items[1]);
			Assert::AreEqual(queue[1], items[0]);
			Assert::ExpectException<std::exception>([&] { queue[2]; });

			// Pop One Again
			hasSizeChanged = queue.Pop();
			Assert::IsTrue(hasSizeChanged);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 1U);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 1U);

			Assert::AreEqual(queue.Back(), items[1]);
			Assert::ExpectException<std::exception>([&] { queue.Back(1); });
			Assert::AreEqual(queue[0], items[1]);
			Assert::ExpectException<std::exception>([&] { queue[1]; });

			// Pop Second
			hasSizeChanged = queue.Pop();
			Assert::IsTrue(hasSizeChanged);

			// Test State
			Assert::IsTrue(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 0U);

			hasSizeChanged = queue.Pop();
			Assert::IsFalse(hasSizeChanged);

			Assert::ExpectException<std::exception>([&] { queue.PopBack(); });
			Assert::ExpectException<std::exception>([&] { queue.Back(); });
			Assert::ExpectException<std::exception>([&] { queue[0]; });

			// ===== All Elements =====

			// Push #1
			hasSizeChanged = queue.PushBack(items[0], &popped);
			Assert::IsTrue(hasSizeChanged);
			Assert::AreEqual(message, popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 1U);

			Assert::AreEqual(queue.Back(), items[0]);
			Assert::ExpectException<std::exception>([&] { queue.Back(1); });
			Assert::AreEqual(queue[0], items[0]);
			Assert::ExpectException<std::exception>([&] { queue[1]; });

			// Push #2
			hasSizeChanged = queue.PushBack(items[1], &popped);
			Assert::IsTrue(hasSizeChanged);
			Assert::AreEqual(message, popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 2U);

			Assert::AreEqual(queue.Back(), items[0]);
			Assert::AreEqual(queue.Back(1), items[1]);
			Assert::ExpectException<std::exception>([&] { queue.Back(2); });
			Assert::AreEqual(queue[0], items[1]);
			Assert::AreEqual(queue[1], items[0]);
			Assert::ExpectException<std::exception>([&] { queue[2]; });

			// Push #3
			hasSizeChanged = queue.PushBack(items[2], &popped);
			Assert::IsTrue(hasSizeChanged);
			Assert::AreEqual(message, popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 3U);

			Assert::AreEqual(queue.Back(), items[0]);
			Assert::AreEqual(queue.Back(1), items[1]);
			Assert::AreEqual(queue.Back(2), items[2]);
			Assert::ExpectException<std::exception>([&] { queue.Back(3); });
			Assert::AreEqual(queue[0], items[2]);
			Assert::AreEqual(queue[1], items[1]);
			Assert::AreEqual(queue[2], items[0]);
			Assert::ExpectException<std::exception>([&] { queue[3]; });

			// Push #4 (It is Now Full)
			hasSizeChanged = queue.PushBack(items[3], &popped);
			Assert::IsTrue(hasSizeChanged);
			Assert::AreEqual(message, popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsTrue(queue.IsFull());
			Assert::AreEqual(queue.Size(), 4U);

			Assert::AreEqual(queue.Back(), items[0]);
			Assert::AreEqual(queue.Back(1), items[1]);
			Assert::AreEqual(queue.Back(2), items[2]);
			Assert::AreEqual(queue.Back(3), items[3]);
			Assert::ExpectException<std::exception>([&] { queue.Back(4); });
			Assert::AreEqual(queue[0], items[3]);
			Assert::AreEqual(queue[1], items[2]);
			Assert::AreEqual(queue[2], items[1]);
			Assert::AreEqual(queue[3], items[0]);
			Assert::ExpectException<std::exception>([&] { queue[4]; });

			// Push Again (Non-Popping)
			hasSizeChanged = queue.PushBack(items[4]);
			Assert::IsFalse(hasSizeChanged);

			// Test State (Should be unchanged)
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsTrue(queue.IsFull());
			Assert::AreEqual(queue.Size(), 4U);

			Assert::AreEqual(queue.Back(), items[0]);
			Assert::AreEqual(queue.Back(1), items[1]);
			Assert::AreEqual(queue.Back(2), items[2]);
			Assert::AreEqual(queue.Back(3), items[3]);
			Assert::ExpectException<std::exception>([&] { queue.Back(4); });
			Assert::AreEqual(queue[0], items[3]);
			Assert::AreEqual(queue[1], items[2]);
			Assert::AreEqual(queue[2], items[1]);
			Assert::AreEqual(queue[3], items[0]);
			Assert::ExpectException<std::exception>([&] { queue[4]; });

			// Push #5 (It is Now Overflowing)
			hasSizeChanged = queue.PushBack(items[4], &popped);
			Assert::IsFalse(hasSizeChanged);
			Assert::AreNotEqual(message, popped);
			Assert::AreEqual(items[0], popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsTrue(queue.IsFull());
			Assert::AreEqual(queue.Size(), 4U);

			Assert::AreEqual(queue.Back(), items[1]);
			Assert::AreEqual(queue.Back(1), items[2]);
			Assert::AreEqual(queue.Back(2), items[3]);
			Assert::AreEqual(queue.Back(3), items[4]);
			Assert::ExpectException<std::exception>([&] { queue.Back(4); });
			Assert::AreEqual(queue[0], items[4]);
			Assert::AreEqual(queue[1], items[3]);
			Assert::AreEqual(queue[2], items[2]);
			Assert::AreEqual(queue[3], items[1]);
			Assert::ExpectException<std::exception>([&] { queue[4]; });

			// PopBack
			popped = queue.PopBack();
			Assert::AreEqual(items[1], popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 3U);

			Assert::AreEqual(queue.Back(), items[2]);
			Assert::AreEqual(queue.Back(1), items[3]);
			Assert::AreEqual(queue.Back(2), items[4]);
			Assert::ExpectException<std::exception>([&] { queue.Back(3); });
			Assert::AreEqual(queue[0], items[4]);
			Assert::AreEqual(queue[1], items[3]);
			Assert::AreEqual(queue[2], items[2]);
			Assert::ExpectException<std::exception>([&] { queue[3]; });

			// Push #5 (Re-Fill)
			popped = message;
			hasSizeChanged = queue.PushBack(items[5], &popped);
			Assert::IsTrue(hasSizeChanged);
			Assert::AreEqual(message, popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsTrue(queue.IsFull());
			Assert::AreEqual(queue.Size(), 4U);

			Assert::AreEqual(queue.Back(), items[2]);
			Assert::AreEqual(queue.Back(1), items[3]);
			Assert::AreEqual(queue.Back(2), items[4]);
			Assert::AreEqual(queue.Back(3), items[5]);
			Assert::ExpectException<std::exception>([&] { queue.Back(4); });
			Assert::AreEqual(queue[0], items[5]);
			Assert::AreEqual(queue[1], items[4]);
			Assert::AreEqual(queue[2], items[3]);
			Assert::AreEqual(queue[3], items[2]);
			Assert::ExpectException<std::exception>([&] { queue[4]; });

			// Push #6 (Re-Overflowing)
			popped = message;
			hasSizeChanged = queue.PushBack(items[6], &popped);
			Assert::IsFalse(hasSizeChanged);
			Assert::AreNotEqual(message, popped);
			Assert::AreEqual(items[2], popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsTrue(queue.IsFull());
			Assert::AreEqual(queue.Size(), 4U);

			Assert::AreEqual(queue.Back(), items[3]);
			Assert::AreEqual(queue.Back(1), items[4]);
			Assert::AreEqual(queue.Back(2), items[5]);
			Assert::AreEqual(queue.Back(3), items[6]);
			Assert::ExpectException<std::exception>([&] { queue.Back(4); });
			Assert::AreEqual(queue[0], items[6]);
			Assert::AreEqual(queue[1], items[5]);
			Assert::AreEqual(queue[2], items[4]);
			Assert::AreEqual(queue[3], items[3]);
			Assert::ExpectException<std::exception>([&] { queue[4]; });

			// PopBack
			popped = queue.PopBack();
			Assert::AreEqual(items[3], popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 3U);

			Assert::AreEqual(queue.Back(), items[4]);
			Assert::AreEqual(queue.Back(1), items[5]);
			Assert::AreEqual(queue.Back(2), items[6]);
			Assert::ExpectException<std::exception>([&] { queue.Back(3); });
			Assert::AreEqual(queue[0], items[6]);
			Assert::AreEqual(queue[1], items[5]);
			Assert::AreEqual(queue[2], items[4]);
			Assert::ExpectException<std::exception>([&] { queue[3]; });

			// PopBack
			popped = queue.PopBack();
			Assert::AreEqual(items[4], popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 2U);

			Assert::AreEqual(queue.Back(), items[5]);
			Assert::AreEqual(queue.Back(1), items[6]);
			Assert::ExpectException<std::exception>([&] { queue.Back(2); });
			Assert::AreEqual(queue[0], items[6]);
			Assert::AreEqual(queue[1], items[5]);
			Assert::ExpectException<std::exception>([&] { queue[2]; });

			// PopBack
			popped = queue.PopBack();
			Assert::AreEqual(items[5], popped);

			// Test State
			Assert::IsFalse(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 1U);

			Assert::AreEqual(queue.Back(), items[6]);
			Assert::ExpectException<std::exception>([&] { queue.Back(1); });
			Assert::AreEqual(queue[0], items[6]);
			Assert::ExpectException<std::exception>([&] { queue[1]; });

			// PopBack
			popped = queue.PopBack();
			Assert::AreEqual(items[6], popped);

			// Test State
			Assert::IsTrue(queue.IsEmpty());
			Assert::IsFalse(queue.IsFull());
			Assert::AreEqual(queue.Size(), 0U);

			Assert::ExpectException<std::exception>([&] { queue.Back(); });
			Assert::ExpectException<std::exception>([&] { queue[0]; });

			// PopBack
			Assert::ExpectException<std::exception>([&] { queue.PopBack(); });

			hasSizeChanged = queue.Pop();
			Assert::IsFalse(hasSizeChanged);
		}
	};
}/**/