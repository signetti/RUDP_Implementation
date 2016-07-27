#pragma once
#include <chrono>

class IManager
{
protected:
	virtual void Initialize() = 0;

public:
	virtual void Draw() = 0;
	virtual void Update(const std::chrono::milliseconds& deltaTime) = 0;
	virtual void Shutdown() = 0;
};