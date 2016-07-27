#include "stdafx.h"

#include "Simulation.h"

#include "EntityManager.h"
#include "NetworkManager.h"
#include "BallEntity.h"
#include "FieldEntity.h"
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
		Coord2D position;
		Coord2D velocity;
		for (int ballCount = 0; ballCount < 2; ++ballCount)
		{
			velocity.x = getRangedRandom(-20.0f, 20.0f);
			velocity.y = getRangedRandom(-20.0f, 20.0f);

			id = GenerateID();
			EntityManager::GetInstance()->AddEntity(new BallEntity(id, 20.0f, position, velocity, RGB(0xBB, 0xEE, 0xEE)));
			NetworkManager::GetInstance()->SendBallAction(id, EActionType::EActionType_SPAWN, 10.0f, position, velocity, RGB(0xBB, 0xEE, 0xEE));
		}
	}
	else
	{	// Establish Connection / Initialize Manager
		NetworkManager::CreateClientInstance(std::chrono::milliseconds(10));
	}
}

void Simulation::UpdateFrame(std::chrono::milliseconds deltaTime)
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
