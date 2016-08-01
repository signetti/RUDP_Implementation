#pragma once
#include "Entity.h"
#include "baseTypes.h"

class BallEntity : public Entity
{
private:
	uint32_t	mBallColor;
	float		mRadius;
	Coord2D mPosition;
	Coord2D mVelocity;
	uint32_t mLifetime;

private:
	void CollideField();

public:
	BallEntity(id_number id, float radius, const Coord2D& initPos, const Coord2D& initVel, uint32_t initColor);
	~BallEntity();

	void setBallColor(uint32_t color) { mBallColor = color; };
	void setBallColor(uint8_t r, uint8_t g, uint8_t b);

public:
	void Draw() override;
	void Update(const millisecond& deltaTime) override;
	uint32_t GetLifetime() const;

	void Move(uint32_t color, float radius, const Coord2D& position, const Coord2D& velocity);
};

