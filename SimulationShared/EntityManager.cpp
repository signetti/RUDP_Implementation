#include "stdafx.h"
//#include <assert.h>
#include "random.h"

#include "EntityManager.h"

#include "Entity.h"
#include "BallEntity.h"
#include "FieldEntity.h"

#undef min
#include "action_generated.h"
#include "NetworkManager.h"
#include "Simulation.h"

#define TURN_OFF_SPAWN true

EntityManager* EntityManager::sInstance = nullptr;

EntityManager::~EntityManager()
{
	Shutdown();
}

void EntityManager::CreateInstance()
{
	if (sInstance == nullptr)
	{
		sInstance = new EntityManager();
		sInstance->Initialize();
	}
	else
	{
		throw std::exception("Cannot create more than one EntityManager.");
	}
}

void EntityManager::AddEntity(Entity* entity)
{
	if (entity != nullptr)
	{
		// Insert if it does not exist
		auto key = entity->ID();
		auto found = mEntityList.lower_bound(key);
		if (found == mEntityList.end())
		{
			mEntityList.insert(found, std::pair<id_number, Entity* >(key, entity));
			++mNumberOfSpawns;	// Increment number of spawns
		}
		else
		{
			delete entity;
			//throw std::exception("Duplicate Entity being created");
		}
	}
}

bool EntityManager::RemoveEntityByID(id_number id)
{
	auto found = mEntityList.find(id);
	bool doesExist = (found != mEntityList.end());
	if (doesExist)
	{
		mEntityList.erase(found);
		++mNumberOfDestroys;
	}
	return doesExist;
}

Entity * EntityManager::GetEntityByID(id_number id)
{
	auto found = mEntityList.find(id);
	if (found != mEntityList.end())
	{
		return found->second;
	}
	else
	{
		return nullptr;
	}
}

uint32_t EntityManager::NumberOfEntities() const
{
	return static_cast<uint32_t>(mEntityList.size());
}

void EntityManager::Draw()
{
	for (auto& entity : mEntityList)
	{
		entity.second->Draw();
	}
}

void EntityManager::Update(const millisecond& deltaTime)
{
	std::vector<id_number> ballsToRemove;

	// Update Balls
	for (auto& element : mEntityList)
	{
		auto& id = element.first;
		BallEntity* entity = reinterpret_cast<BallEntity*>(element.second);

		entity->Update(deltaTime);

		// Remove Ball over lifetime
		if (id != 0 && entity->GetLifetime() >= getRangedRandom(3000, 5000))
		{
			ballsToRemove.push_back(id);
		}
	}

#if !TURN_OFF_SPAWN
	// Remove Balls
	for (auto& id : ballsToRemove)
	{
		bool success = RemoveEntityByID(id);
		//assert(success);	// No reason to fail
		NetworkManager::GetInstance()->SendBallAction(id, EActionType::EActionType_DESTROY, 0, Coord2D(), Coord2D(), 0);
	}

	if (mEntityList.size() < static_cast<uint32_t>(getRangedRandom(20, 50)))
	{
		reinterpret_cast<Simulation*>(Simulation::GetInstance())->CreateRandomBall();
	}
#endif
}

void EntityManager::Shutdown()
{
	for (auto& entity : mEntityList)
	{
		free(entity.second);
	}
	mEntityList.clear();
}
