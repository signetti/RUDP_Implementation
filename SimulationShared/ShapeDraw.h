#ifndef _NO_DRAW
void DrawCircle(float_t radius, float_t x, float_t y, uint8_t r, uint8_t g, uint8_t b, bool filled);
void DrawLine(float_t startX, float_t startY, float_t endX, float_t endY, uint8_t r, uint8_t g, uint8_t b);
void DrawRectangle(float_t upperLeftX, float_t upperRightX, float_t lowerLeftX, float_t lowerRightX,
				   float_t upperLeftY, float_t upperRightY, float_t lowerLeftY, float_t lowerRightY,
				   uint8_t r, uint8_t g, uint8_t b);
#endif