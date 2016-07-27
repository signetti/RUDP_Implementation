#include "stdafx.h"

#include "BallEntity.h"
#include "FieldEntity.h"
#include "ShapeDraw.h"

#include "NetworkManager.h"
#undef min
#include "action_generated.h"

BallEntity::BallEntity(id_number id, float radius, const Coord2D& initPos, const Coord2D& initVel, uint32_t initColor) : Entity(id)
	, mRadius(radius), mPosition(initPos), mVelocity(initVel), mBallColor(initColor) {}

BallEntity::~BallEntity() {}

void BallEntity::setBallColor(uint8_t r, uint8_t g, uint8_t b)
{
	mBallColor = b;
	mBallColor += g << 8;
	mBallColor += r << 16;
}

void BallEntity::Draw()
{
	uint8_t red = (uint8_t)((mBallColor >> 16) & 0xFF);
	uint8_t green = (uint8_t)((mBallColor >> 8) & 0xFF);
	uint8_t blue = (uint8_t)((mBallColor >> 0) & 0xFF);

	DrawCircle(mRadius, mPosition.x, mPosition.y, red, green, blue, false);
}

void BallEntity::Update(const std::chrono::milliseconds& deltaTime)
{
	uint32_t milliseconds = static_cast<uint32_t>(deltaTime.count());

	mPosition.x += mVelocity.x * milliseconds / 10;
	mPosition.y += mVelocity.y * milliseconds / 10;

	CollideField();

	NetworkManager::GetInstance()->SendBallAction(mID, EActionType::EActionType_MOVE, 10.0f, mPosition, mVelocity, mBallColor);
}

void BallEntity::Move(uint32_t color, float radius, const Coord2D & position, const Coord2D & velocity)
{
	mBallColor = color;
	mRadius = radius;
	mPosition = position;
	mVelocity = velocity;
}

void BallEntity::CollideField()
{
	FieldEntity* field = FieldEntity::GetInstance();

	float_t rightSide = ((float_t)field->GetWidth() / 2.0f);
	float_t leftSide = 0 - ((float_t)field->GetWidth() / 2.0f);
	float_t topSide = ((float_t)field->GetHeight() / 2.0f);
	float_t bottomSide = 0 - ((float_t)field->GetHeight() / 2.0f);

	if (mPosition.x - mRadius <= leftSide)
	{
		mVelocity.x = -mVelocity.x;
		mPosition.x = leftSide + mRadius;
	}
	if (mPosition.x + mRadius >= rightSide)
	{
		mVelocity.x = -mVelocity.x;
		mPosition.x = rightSide - mRadius;
	}
	if (mPosition.y - mRadius <= bottomSide)
	{
		mVelocity.y = -mVelocity.y;
		mPosition.y = bottomSide + mRadius;
	}
	if (mPosition.y + mRadius >= topSide)
	{
		mVelocity.y = -mVelocity.y;
		mPosition.y = topSide - mRadius;
	}
};