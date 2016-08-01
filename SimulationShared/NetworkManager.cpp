#include "stdafx.h"

#include "NetworkManager.h"
#include <exception>
#include "Socket.h"

#include "EntityManager.h"
#include "BallEntity.h"
#include "FieldEntity.h"

#include "ConfigReader.h"
#include "SharedTestData.h"
#include "TCPSocket.h"
#include "RUDPSocket.h"
#include "baseTypes.h"

#include "Entity.h"
#undef min
#include "action_generated.h"

NetworkManager* NetworkManager::sInstance = nullptr;

NetworkManager::NetworkManager() : mSocket(nullptr), mTimeDuration(), bIsServer(true) {}
NetworkManager::NetworkManager(const std::chrono::milliseconds& reservedTime) : mSocket(nullptr), mTimeDuration(reservedTime), bIsServer(false) {}

void NetworkManager::CreateServerInstance()
{
	Logger::SetLoggerState(__FILE__, true, false, BasicColor::WHITE);
	if (sInstance == nullptr)
	{
		sInstance = new NetworkManager();
		sInstance->Initialize();
	}
	else
	{
		throw std::exception("Cannot create more than one EntityManager.");
	}
}

void NetworkManager::CreateClientInstance(const std::chrono::milliseconds& reservedTime)
{
	Logger::SetLoggerState(__FILE__, false, false, BasicColor::WHITE);
	if (sInstance == nullptr)
	{
		sInstance = new NetworkManager(reservedTime);
		sInstance->Initialize();
	}
	else
	{
		throw std::exception("Cannot create more than one EntityManager.");
	}
}

#ifdef config
static_assert("Config already defined... conflict!");
#endif
#define config config	// Simply used for identifying config's effect in code (using Visual Studio text color)
void NetworkManager::Initialize()
{
	// ========================== Parse Config File =========================
	config_t config;
	if (!ConfigReader::ParseConfig(config, CONFIG_FILE_PATH))
	{
		throw std::exception("Parsing Config file Failed.");
	}

	if (bIsServer)
	{	// ============== Initialize Server Connection ==============
		// Create a server that is listening to the defined-port
		switch (config.protocol)
		{
		case EProtocol::TCP:
			mServer = std::shared_ptr<IServerSocket>(new TCPServerSocket(DEFAULT_SERVER_PORT_NUMBER));
			break;
		case EProtocol::UDP:
			mServer = std::shared_ptr<IServerSocket>(new UDPServerSocket(DEFAULT_SERVER_PORT_NUMBER, config.maxTimeoutMS));
			break;
		case EProtocol::RUDP:
			mServer = std::shared_ptr<IServerSocket>(new RUDPServerSocket(DEFAULT_SERVER_PORT_NUMBER, config.maxTimeoutMS));
			break;
		}


		// Open the server for connection (create socket, bind, then listen)

		// Listen for and accept a client connection
		Logger::PrintF(__FILE__, "Awaiting Client. . . ");
		mSocket = mServer->Accept();

		// Notify that connection is reached
		Logger::PrintF(__FILE__, "Client-Server Connection Established.\n");
	}
	else
	{	// ===================== Begin Connection to Server =====================
		switch (config.protocol)
		{
		case EProtocol::TCP:
			mSocket = new TCPSocket();
			break;
		case EProtocol::UDP:
			mSocket = new UDPSocket(DEFAULT_CLIENT_PORT_NUMBER, config.maxTimeoutMS);
			break;
		case EProtocol::RUDP:
			mSocket = new RUDPSocket(DEFAULT_CLIENT_PORT_NUMBER, config.maxTimeoutMS);
			break;
		}

		bool isSuccess = mSocket->Connect(config.serverAddress, DEFAULT_SERVER_PORT_NUMBER);
		if (!isSuccess)
		{
			throw std::exception("Error in creating Client-Server Connection.\n");
		}
	}
}
#undef config

void NetworkManager::ParseAction(uint8_t * buffer, uint32_t bufferSize)
{
	auto verifier = flatbuffers::Verifier(buffer, bufferSize);
	bool isTrusted = VerifyActionBuffer(verifier);

	if (isTrusted)
	{
		const Action& action = *(GetAction(buffer));

		switch (action.ActionType())
		{
		case EActionType::EActionType_SPAWN:
			{	// Spawn
				switch (action.Data_type())
				{
				case ClassData::ClassData_BallData:
					{
						auto& data = *(reinterpret_cast<const BallData *>(action.Data()));

						Coord2D pos(data.Position()->x(), data.Position()->y());
						Coord2D vel(data.Velocity()->x(), data.Velocity()->y());

						EntityManager::GetInstance()->AddEntity(new BallEntity(action.ID(), data.Radius(), pos, vel, data.Color()));
						break;
					}
				case ClassData::ClassData_FieldData:
					{
						auto& data = *(reinterpret_cast<const FieldData *>(action.Data()));
						EntityManager::GetInstance()->AddEntity(new FieldEntity(action.ID(), data.Width(), data.Height(), data.Color()));
						break;
					}
				}
				break;
			}
		case  EActionType::EActionType_MOVE:
			{	// Move
				switch (action.Data_type())
				{
				case ClassData::ClassData_BallData:
					{
						auto& data = *(reinterpret_cast<const BallData *>(action.Data()));
						auto entity = reinterpret_cast<BallEntity*>(EntityManager::GetInstance()->GetEntityByID(action.ID()));

						if (entity != nullptr)
						{
							Coord2D pos(data.Position()->x(), data.Position()->y());
							Coord2D vel(data.Velocity()->x(), data.Velocity()->y());

							entity->Move(data.Color(), data.Radius(), pos, vel);
						}

						break;
					}
				case ClassData::ClassData_FieldData:
					{
						auto& data = *(reinterpret_cast<const FieldData *>(action.Data()));
						auto entity = reinterpret_cast<FieldEntity*>(EntityManager::GetInstance()->GetEntityByID(action.ID()));

						if (entity != nullptr)
						{
							entity->Move(data.Color(), data.Width(), data.Height());
						}
						break;
					}
				}
				break;
			}
		case  EActionType::EActionType_DESTROY:
			{	// Destroy
				EntityManager::GetInstance()->RemoveEntityByID(action.ID());
				break;
			}
		}
	}
}

char * NetworkManager::ActionTypeToChar(EActionType actionType)
{
	switch (actionType)
	{
	case EActionType::EActionType_SPAWN:
		return "Spawn";
	case EActionType::EActionType_MOVE:
		return "Move";
	case EActionType::EActionType_DESTROY:
		return "Destroy";
	default:
		return "ERROR";
	}
}

void NetworkManager::SendBallAction(id_number id, EActionType actionType, float radius, Coord2D position, Coord2D velocity, uint32_t color)
{
	if (bIsServer)
	{
		// Serialize Action Data
		uint8_t * packet_serialized;
		uint32_t packet_size;

		flatbuffers::FlatBufferBuilder builder;
		auto pos = Point2D(position.x, position.y);
		auto vel = Point2D(velocity.x, velocity.y);

		auto ball = CreateBallData(builder, &pos, &vel, color, radius);
		auto packet = CreateAction(builder, id, ClassData::ClassData_BallData, ball.Union(), actionType);

		builder.Finish(packet);

		packet_serialized = builder.GetBufferPointer();
		packet_size = builder.GetSize();

		// Send Action Data
		switch (actionType)
		{
		case EActionType::EActionType_SPAWN:
		case EActionType::EActionType_DESTROY:
			// High Priority! Complete on Success
			while (mSocket->Send(packet_serialized, packet_size) != true)
			{	// Sleep for a while
				Sleep(1);
			}
			break;
		default:
			// Low Priority! Fire-and-Forget...
			mSocket->Send(packet_serialized, packet_size);
			break;
		}

	//	Logger::PrintF(__FILE__, "Action %s Sent to Ball #%d: rad<%.02f> pos<%.02f,%.02f> vel<%.02f,%.02f>\n"
	//		, ActionTypeToChar(actionType), id, radius, position.x, position.y, velocity.x, velocity.y);
	}
}

void NetworkManager::SendFieldAction(id_number id, EActionType actionType, uint32_t width, uint32_t height, uint32_t color)
{
	if (bIsServer)
	{
		// Serialize Action Data
		uint8_t * packet_serialized;
		uint32_t packet_size;

		flatbuffers::FlatBufferBuilder builder;

		auto field = CreateFieldData(builder, width, height, color);
		auto packet = CreateAction(builder, id, ClassData::ClassData_FieldData, field.Union(), actionType);

		builder.Finish(packet);

		packet_serialized = builder.GetBufferPointer();
		packet_size = builder.GetSize();

		// Send Action Data
		// High Priority! Complete on Success
		while (mSocket->Send(packet_serialized, packet_size) != true)
		{	// Sleep for a while
			Sleep(1);
		}

	//	Logger::PrintF(__FILE__, "Action %s Sent to Field #%d: width<%.02f> height<%.02f>\n"
	//		, ActionTypeToChar(actionType), id, width, height);
	}
}

void NetworkManager::Update(const millisecond&)
{
	if (!bIsServer)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
	
		uint8_t buffer[1200U];
		uint32_t bytesReceived;
		do
		{
			bytesReceived = mSocket->Receive(buffer, 1200U);

			if (bytesReceived > 0)
			{
				ParseAction(buffer, bytesReceived);
			}
			else
			{
				break;
			}
		}
		while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime) < mTimeDuration);
	}
}



void NetworkManager::Shutdown() {}
