// ServerSimulation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <chrono>

#define _NO_DRAW
#include "Simulation.h"

int main()
{
	IGame* game = IGame::CreateInstance<Simulation>();

	reinterpret_cast<Simulation*>(game)->SetAsServer(true);

	game->Initialize();

	auto startTime = std::chrono::high_resolution_clock::now();
	auto endTime = startTime;
	std::chrono::milliseconds elapsedTime(0);

	WORD millisecondsPerFrame = 33;
	int32_t checkFrameTime;
	for (;;)
	{
		// Update Frame
		game->UpdateFrame(elapsedTime);

		// Elapsed Time for Update
		endTime = std::chrono::high_resolution_clock::now();
		elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		// Sleep for a given frame duration
		checkFrameTime = millisecondsPerFrame - (uint32_t)elapsedTime.count();
		if (checkFrameTime > 0)
		{
			Sleep((WORD)checkFrameTime);
		}

		// Elapsed Time total
		endTime = std::chrono::high_resolution_clock::now();
		elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
		startTime = endTime;
	}
}

