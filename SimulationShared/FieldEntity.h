#pragma once
#include "Entity.h"

class FieldEntity :	public Entity
{
private:
	uint32_t mColor;
	uint32_t mHeight;
	uint32_t mWidth;

	static FieldEntity * sInstance;
	FieldEntity(uint32_t width, uint32_t height, uint32_t initColor);
	~FieldEntity() = default;

public:
	FieldEntity(id_number id, uint32_t width, uint32_t height, uint32_t color);
	static FieldEntity* CreateInstance(id_number id, uint32_t width, uint32_t height, uint32_t initColor);
	static FieldEntity* GetInstance()		{ return sInstance; }


	void SetColor(uint32_t color)	{ mColor = color; };
	void SetWidth(uint32_t width)	{ mWidth = width; };
	uint32_t GetWidth()				{ return mWidth; };
	void SetHeight(uint32_t height)	{ mHeight = height; };
	uint32_t GetHeight()			{ return mHeight; };

	void Draw() override;
	void Update(const std::chrono::milliseconds& deltaTime) override;

	void Move(uint32_t color, uint32_t width, uint32_t height);
};

