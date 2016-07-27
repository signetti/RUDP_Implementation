#include "stdafx.h"

#define OPENGLSTUFF_CPP

#include <windows.h>											// Header File For Windows
#include <stdio.h>												// Header File For Standard Input / Output
#include <stdarg.h>												// Header File For Variable Argument Routines
#include <math.h>												// Header File For Math Operations
#include <gl\gl.h>												// Header File For The OpenGL32 Library
#include <gl\glu.h>		
#include <gl\glut.h>
#include "gameFramework.h"
#include "openglstuff.h"

void initOpenGLDrawing( GL_Window *window, Keys *keys, float backRed, float backGreen, float backBlue)
{
	glClearColor(backRed, backGreen, backBlue, 0.0f);			// Background Color
	glClearDepth(1.0f);											// Depth Buffer Setup
	glDepthFunc(GL_LEQUAL);										// Type Of Depth Testing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);			// Enable Alpha Blending
	//glEnable(GL_DEPTH_TEST);									// Enable Depth Testing
	glEnable(GL_BLEND);											// Enable Blending
	glEnable(GL_TEXTURE_2D);									// Enable Texture Mapping
	glEnable(GL_CULL_FACE);										// Remove Back Face
	
	g_window	= window;
	g_keys		= keys;
}
void endOpenGLDrawing()
{
	glFlush();
}

void startOpenGLDrawing()
{
		// Clear the window
	glClear(GL_COLOR_BUFFER_BIT);
	// Set the modelview matrix to be the identity matrix
	glLoadIdentity();
}

BOOL Initialize(GL_Window* window, Keys* keys)					// Any OpenGL Initialization Goes Here
{
	initOpenGLDrawing(window, keys, 0.0f, 0.0f, 0.0f);
	return TRUE;
}