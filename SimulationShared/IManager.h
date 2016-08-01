#pragma once
#include "baseTypes.h"

class IManager
{
protected:
	virtual void Initialize() = 0;

public:
	virtual void Draw() = 0;
	virtual void Update(const millisecond& deltaTime) = 0;
	virtual void Shutdown() = 0;
};