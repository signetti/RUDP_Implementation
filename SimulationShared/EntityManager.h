#pragma once
#include "IManager.h"
#include <map>

typedef uint32_t id_number;
class Entity;

class EntityManager : public IManager
{
private:
	std::map<id_number, Entity*> mEntityList;

	static EntityManager* sInstance;

	uint32_t mNumberOfSpawnRequests;
	uint32_t mNumberOfDestroyRequests;

	uint32_t mNumberOfSpawns;
	uint32_t mNumberOfDestroys;

protected:
	EntityManager() = default;
	~EntityManager();
	void Initialize() override {}

public:
	static void CreateInstance();
	static EntityManager* GetInstance() { return sInstance; }

	void AddEntity(Entity* entity);
	bool RemoveEntityByID(id_number id);
	Entity * GetEntityByID(id_number id);

	uint32_t NumberOfEntities() const;
	uint32_t NumberOfSpawns() const		{ return mNumberOfSpawns; }
	uint32_t NumberOfDestroys() const	{ return mNumberOfDestroys; }
	uint32_t NumberOfSpawnRequests() const		{ return mNumberOfSpawnRequests; }
	uint32_t NumberOfDestroyRequests() const	{ return mNumberOfDestroyRequests; }

	void Draw() override;
	void Update(const millisecond& deltaTime) override;
	void Shutdown() override;
};

