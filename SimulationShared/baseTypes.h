#pragma once
#include <cstdint>

struct Coord2D
{
    float x;
    float y;
	Coord2D() : x(), y() {}
	Coord2D(float x, float y) : x(x), y(y) {}
	Coord2D(const Coord2D& coord) : x(coord.x), y(coord.y) {}
};

typedef uint32_t id_number;
/*
struct Color
{
	union
	{
		uint32_t R_G_B;
		struct
		{
			uint8_t R;
			uint8_t G;
			uint8_t B;
		};
	};

	Color(uint32_t color) : R_G_B(color) {}
	Color(uint8_t red, uint8_t green, uint8_t blue) {}
};*/