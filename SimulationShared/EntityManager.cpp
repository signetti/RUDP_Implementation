#include "stdafx.h"

#include "EntityManager.h"
#include "BallEntity.h"
#include "FieldEntity.h"

#include "NetworkManager.h"
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
		}
		else
		{
			throw std::exception("Duplicate Entity being created");
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

void EntityManager::Draw()
{
	for (auto& entity : mEntityList)
	{
		entity.second->Draw();
	}
}

void EntityManager::Update(const std::chrono::milliseconds& deltaTime)
{
	(deltaTime);
	
	for (auto& entity : mEntityList)
	{
		entity.second->Update(deltaTime);
	}/**/
}

void EntityManager::Shutdown()
{
	for (auto& entity : mEntityList)
	{
		free(entity.second);
	}
	mEntityList.clear();
}
