#pragma once
#include "IManager.h"
#include "baseTypes.h"
#include <memory>
#include <chrono>

class Socket;
class IServerSocket;
enum EActionType;

class NetworkManager : public IManager
{
private:
	static NetworkManager* sInstance;

	std::shared_ptr<IServerSocket> mServer;
	Socket* mSocket;
	std::chrono::milliseconds mTimeDuration;
	bool bIsServer;

	NetworkManager();
	NetworkManager(const std::chrono::milliseconds& reservedTime);

	void Initialize() override;
	void ParseAction(uint8_t* buffer, uint32_t bufferSize);
	
	static char * ActionTypeToChar(EActionType actionType);

public:
	static void CreateServerInstance();
	static void CreateClientInstance(const std::chrono::milliseconds& reservedTime);
	static NetworkManager* GetInstance() { return sInstance; }

	void Draw() override {}
	void Update(const millisecond& deltaTime) override;
	void Shutdown() override;

	void SendBallAction(id_number id, EActionType actionType, float radius, Coord2D position, Coord2D velocity, uint32_t color);
	void SendFieldAction(id_number id, EActionType actionType, uint32_t width, uint32_t height, uint32_t color);
};

