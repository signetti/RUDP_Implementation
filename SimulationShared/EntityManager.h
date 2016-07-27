#pragma once
#include "IManager.h"
#include "Entity.h"
#include <map>

class EntityManager : public IManager
{
private:
	std::map<id_number, Entity*> mEntityList;

	static EntityManager* sInstance;

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

	void Draw() override;
	void Update(const std::chrono::milliseconds& deltaTime) override;
	void Shutdown() override;
};

