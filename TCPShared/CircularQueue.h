#pragma once
#include <assert.h>
template <typename T, uint32_t QueueSize>
class CircularQueue
{
private:
	T mQueue[QueueSize];
	std::uint32_t mEndIndex;
	std::uint32_t mSize;
	//std::uint32_t mQueueSize;

public:
	static const uint32_t MaximumQueueSize = QueueSize;

	CircularQueue() : mEndIndex(0), mSize(0) {}

	// Return the object at the given index, where 0 is the oldest pushed in the queue.
	T& operator[](uint32_t index)
	{
		if (index >= mSize)
		{
			throw std::exception("Index Out of Bound");
		}
		uint32_t lastIndex = ((QueueSize + (mEndIndex - index - 1)) % QueueSize);
		assert(lastIndex >= 0 && lastIndex < QueueSize);
		return mQueue[lastIndex];
	}
	const T& operator[](uint32_t index) const { return const_cast<CircularQueue*>(this)->operator[](index); }

	T& Back(const uint32_t& offsetFromEnd = 0U)
	{
		// Check for valid index
		if (offsetFromEnd >= mSize)
		{
			throw std::exception("Index Out of Bound");
		}
		// Solve for proper index
		uint32_t properIndex = (QueueSize + (mEndIndex - mSize) + offsetFromEnd) % QueueSize;
		// Access proper Index
		assert(properIndex >= 0 && properIndex < QueueSize);
		return mQueue[properIndex];
	}
	const T& Back(const uint32_t& offsetFromEnd = 0U) const { return const_cast<CircularQueue*>(this)->Back(offsetFromEnd); }

	// Returns true if the size has changed
	bool Pop()
	{
		if (mSize == 0)
		{
			return false;
		}
		else
		{
			--mSize;
			return true;
		}
	}

	T PopBack()
	{
		if (mSize == 0)
		{
			throw std::exception("Invalid Operation: Pop an empty Queue");
		}

		// Solve for proper index
		uint32_t properIndex = (QueueSize + mEndIndex - mSize) % QueueSize;
		T popped = mQueue[properIndex];

		--mSize;

		return popped;
	}

	// Returns true if size has changed, false otherwise
	bool PushBack(const T& item, T* OutPopped = nullptr)
	{
		bool result = true;
		if (mSize >= QueueSize)
		{	// Queue Overflowed, handle condition. . .
			if (OutPopped == nullptr)
			{	// Return Failed
				return false;
			}
			else
			{	// Return value overwritten in OutPopped
				uint32_t beginningIndex = (QueueSize + (mEndIndex - mSize)) % QueueSize;
				*(OutPopped) = std::move(mQueue[beginningIndex]);
				--mSize;

				// Return false after Pop procedure
				result = false;
			}
		}

		// Perform Push
		mQueue[mEndIndex] = item;

		++mSize;
		++mEndIndex;
		if (mEndIndex >= QueueSize)
		{
			mEndIndex -= QueueSize;
		}

		return result;
	}

	bool IsEmpty() const { return mSize == 0; }
	bool IsFull() const { return mSize == QueueSize; }
	const std::uint32_t Size() const { return mSize; }
};
