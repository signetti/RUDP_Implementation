#pragma once
#include <ctime>
#include <vector>
#include <chrono>
#include <functional>

/*	Time Recorder
	This is used to store the results of the stop-watch.
	The unit of time is modifiable through a template.
	See StopWatch class.
*/
template <typename UnitTime = std::chrono::milliseconds>
class TimeRecorder
{
private:
	std::vector<UnitTime> mLapTimeElapsed;

public:
	TimeRecorder() : mLapTimeElapsed() {}
	~TimeRecorder() {}

	void LapTime(const UnitTime& lapTime)
	{
		mLapTimeElapsed.push_back(lapTime);
	}

	std::vector<UnitTime> GetTimeRecordings(bool clearOnRetrieve = false)
	{
		if (clearOnRetrieve)
		{
			return std::move(mLapTimeElapsed);
		}
		else
		{
			return mLapTimeElapsed;
		}
	}

	const UnitTime& GetLastTimeRecorded()
	{
		return mLapTimeElapsed.back();
	}
};

/*	StopWatch
	Designed to call the stopwatch on construction, and end on destruction.
	The result is saved on the TimeRecorder passed in on construction.
	The unit of time is modifiable through a template.

	The idea and assistance originated from this site:
	https://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=156
*/
template <typename UnitTime = std::chrono::milliseconds>
class StopWatch
{
private:
	std::chrono::high_resolution_clock::time_point mStartTime;
	TimeRecorder<UnitTime> * mRecorderRef;
	static const double CLOCKS_PER_MICROSECONDS;
public:
	StopWatch(TimeRecorder<UnitTime>& recorder) : mRecorderRef(&recorder), mStartTime(std::chrono::high_resolution_clock::now()) 	{}
	~StopWatch() 
	{
		std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
		UnitTime elapsedTime = std::chrono::duration_cast<UnitTime>(endTime - mStartTime);
		mRecorderRef->LapTime(elapsedTime);
	}
};
template <typename UnitTime>
const double StopWatch<UnitTime>::CLOCKS_PER_MICROSECONDS = (double(CLOCKS_PER_SEC / 1000000) == 0) ? 1 : double(CLOCKS_PER_SEC / 1000000);