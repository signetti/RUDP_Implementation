#pragma once
#include "IGame.h"
#include "Entity.h"

class Simulation : public IGame
{
protected:
	friend IGame;

	static id_number sEntityIDGenerator;

	bool bIsServer;

	Simulation() : bIsServer(false) {}
	~Simulation() = default;

	void Initialize() override;
	id_number GenerateID() { return sEntityIDGenerator++; }

public:
	void SetAsServer(bool isServer) { bIsServer = isServer; }

	void UpdateFrame(std::chrono::milliseconds deltaTime) override;
	void DrawScene() override;
	void Shutdown() override;

	const char* GetGameTitle() override;
	void GetScreenData(int32_t& outScreenWidth, int32_t& outScreenHeight, int32_t& outBitsPerPixel) override;
};

