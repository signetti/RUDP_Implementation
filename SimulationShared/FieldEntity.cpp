#include "stdafx.h"

#include "FieldEntity.h"
#include "ShapeDraw.h"
#include "EntityManager.h"
#include <exception>

FieldEntity * FieldEntity::sInstance = nullptr;

FieldEntity::FieldEntity(id_number id, uint32_t width, uint32_t height, uint32_t color) : Entity(id)
	, mWidth(width), mHeight(height), mColor(color) {}

FieldEntity * FieldEntity::CreateInstance(id_number id, uint32_t width, uint32_t height, uint32_t color)
{
	if (sInstance != nullptr)
	{
		throw std::exception("Cannot create more than one Field Entity.");
	}
	sInstance = new FieldEntity(id, width, height, color);

	EntityManager::GetInstance()->AddEntity(sInstance);

	return sInstance;
}

void FieldEntity::Draw()
{
	float_t left = 0 - (float_t)mWidth / 2.0f;
	float_t right = (float_t)mWidth / 2.0f;
	float_t bottom = 0 - (float_t)mHeight / 2.0f;
	float_t top = (float_t)mHeight / 2.0f;

	uint8_t r = (uint8_t)(mColor >> 16 & 0xFF);
	uint8_t g = (uint8_t)(mColor >> 8 & 0xFF);
	uint8_t b = (uint8_t)(mColor >> 0 & 0xFF);

	DrawLine(left, top, right, top, r, g, b);
	DrawLine(right, top, right, bottom, r, g, b);
	DrawLine(right, bottom, left, bottom, r, g, b);
	DrawLine(left, bottom, left, top, r, g, b);
}

void FieldEntity::Update(const millisecond&) {}

void FieldEntity::Move(uint32_t color, uint32_t width, uint32_t height)
{
	mColor = color;
	mWidth = width;
	mHeight = height;
}
