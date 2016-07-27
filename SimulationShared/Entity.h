#pragma once
#include <chrono>

#include "baseTypes.h"

class Entity
{
protected:
	std::chrono::high_resolution_clock::time_point mLatestActionTimestamp;
	id_number mID;

	Entity(id_number id) : mID(id) {}

public:
	id_number ID() const { return mID; }
	virtual void Draw() = 0;
	virtual void Update(const std::chrono::milliseconds& deltaTime) = 0;
};