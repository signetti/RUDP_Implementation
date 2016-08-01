#include "stdafx.h"

#include "Simulation.h"

#include "EntityManager.h"
#include "NetworkManager.h"
#include "BallEntity.h"
#include "FieldEntity.h"
#include "userInterface.h"

#include "random.h"
#include "baseTypes.h"

#undef min
#include "action_generated.h"

id_number Simulation::sEntityIDGenerator = 0;

void Simulation::Initialize()
{
	// Create Managers
	EntityManager::CreateInstance();

	if (bIsServer)
	{	
		// Create Server
		NetworkManager::CreateServerInstance();

		auto id = GenerateID();

		FieldEntity::CreateInstance(id, 3500, 3500, RGB(0xFF, 0xCC, 0x00));
		NetworkManager::GetInstance()->SendFieldAction(id, EActionType::EActionType_SPAWN, 3500, 3500, RGB(0xFF, 0xCC, 0x00));

		// Create Balls
		for (int ballCount = 0; ballCount < 50; ++ballCount)
		{
			CreateRandomBall();
		}
	}
	else
	{	// Establish Connection / Initialize Manager
		NetworkManager::CreateClientInstance(std::chrono::milliseconds(50));
	}
}

void Simulation::CreateRandomBall()
{
	auto id = GenerateID();
	float radius = 30.0f;
	uint32_t color;
	Coord2D position;
	Coord2D velocity;

	velocity.x = getRangedRandom(-10.0f, 10.0f);
	velocity.y = getRangedRandom(-10.0f, 10.0f);
	color = getRangedRandom(0x00888888, 0x00FFFFFF);

	EntityManager::GetInstance()->AddEntity(new BallEntity(id, radius, position, velocity, color));
	NetworkManager::GetInstance()->SendBallAction(id, EActionType::EActionType_SPAWN, radius, position, velocity, color);
}

void Simulation::UpdateFrame(millisecond deltaTime)
{
	// Main Loop
	if (bIsServer)
	{
		EntityManager::GetInstance()->Update(deltaTime);
	}
	else
	{
		NetworkManager::GetInstance()->Update(deltaTime);
	}
}

void Simulation::DrawScene()
{
	// Draw here
	if (!bIsServer)
	{
		EntityManager::GetInstance()->Draw();
		UserInterfaceC::GetInstance()->render();
	}
}

void Simulation::Shutdown()
{
	EntityManager::GetInstance()->Shutdown();
	NetworkManager::GetInstance()->Shutdown();
}

const char * Simulation::GetGameTitle()
{
	return "Simulation";
}

void Simulation::GetScreenData(int32_t & outScreenWidth, int32_t & outScreenHeight, int32_t & outBitsPerPixel)
{
	outScreenWidth = 1024;
	outScreenHeight = 768;
	outBitsPerPixel = 32;
}
