#include "stdafx.h"

#include <windows.h>													// Header File For The Windows Library
#include <gl/gl.h>														// Header File For The OpenGL32 Library
#include <gl/glu.h>														// Header File For The GLu32 Library
#include <gl\glut.h>
#include "gameFramework.h"														// Header File For The NeHeGL Basecode

//handles keyboard processing
void keyProcess(void)
{
	if (g_keys->keyDown[VK_ESCAPE])								// Is ESC Being Pressed?
	{
		TerminateApplication (g_window);						// Terminate The Program
	}

	if (g_keys->keyDown[VK_F1])									// Is F1 Being Pressed?
	{
		ToggleFullscreen (g_window);							// Toggle Fullscreen Mode
	}
}