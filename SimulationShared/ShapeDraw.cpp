#include "stdafx.h"

#ifndef _NO_DRAW

#include <windows.h>											// Header File For Windows
#include "baseTypes.h"
#include <gl\gl.h>												// Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <gl\glut.h>
#include <math.h>
//#include <stdlib.h>
#include <stdio.h>

const float DEG2RAD = 3.14159f/180.0f;

void DrawCircle(float_t radius, float_t x, float_t y, uint8_t r, uint8_t g, uint8_t b, bool filled)
{
	glColor3ub(r, g, b);
	glEnable(GL_POINT_SMOOTH);
	if(!filled)
	{
		// Set the point size
		glPointSize(1.0);
		glBegin(GL_POINTS);
		float_t radiusMax = radius + 2.0f;
		float_t radiusMin = radius - 2.0f;
		for (int i=0; i < 360; i++)
		{
			float degInRad = i*DEG2RAD;
			glVertex2f(x + (cos(degInRad)*radius),y + (sin(degInRad)*radius));
			glVertex2f(x + (cos(degInRad)*radiusMax),y + (sin(degInRad)*radiusMax));
			glVertex2f(x + (cos(degInRad)*radiusMin),y + (sin(degInRad)*radiusMin));
		}
		glEnd();
	}
	else
	{
		glPointSize(radius/2.0f);
		glBegin(GL_POINTS);
		glVertex2f(x, y);
		glEnd();
	}
}

void DrawLine(float_t startX, float_t startY, float_t endX, float_t endY, uint8_t r, uint8_t g, uint8_t b)
{
	glColor3ub(r, g, b);
	// Draw filtered lines
	glEnable(GL_LINE_SMOOTH);

	glBegin(GL_LINE_STRIP);
		glVertex2f(startX, startY);
		glVertex2f(endX, endY);
	glEnd();

}
void DrawRectangle(float_t upperLeftX, float_t upperRightX, float_t lowerLeftX, float_t lowerRightX,
				   float_t upperLeftY, float_t upperRightY, float_t lowerLeftY, float_t lowerRightY,
				   uint8_t r, uint8_t g, uint8_t b)
{
	glColor3ub(r, g, b);
	// Draw filtered lines
	//glEnable(GL_LINE_SMOOTH);

	glBegin(GL_POLYGON);
		glVertex2f(lowerRightX,lowerRightY);
		glVertex2f(lowerLeftX,lowerLeftY);
		glVertex2f(upperLeftX,upperLeftY);
		glVertex2f(upperRightX,upperRightY);

	glEnd();
}
#endif