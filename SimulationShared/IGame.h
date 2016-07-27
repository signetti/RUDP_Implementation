#pragma once
#include <chrono>

#include <windows.h>								// Header File For Windows

class IGame
{
private:
	static IGame *sInstance;

public:
	template <typename GameType>
	static IGame* CreateInstance();
	static IGame* GetInstance() { return sInstance; }

public:
	virtual void Initialize() = 0;
	virtual void UpdateFrame(std::chrono::milliseconds deltaTime) = 0;
	virtual void DrawScene() = 0;
	virtual void Shutdown() = 0;

	virtual const char* GetGameTitle() = 0;
	virtual void GetScreenData(int32_t& outScreenWidth, int32_t& outScreenHeight, int32_t& outBitsPerPixel) = 0;
};

template <typename GameType>
static IGame* IGame::CreateInstance()
{
	if (sInstance == nullptr)
	{
		sInstance = new GameType();
	}
	return sInstance;
}
