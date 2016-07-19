// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//#include <tchar.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>	// Logger.cpp:	For colored windows

#include <winsock2.h>	// For anything networking
#include <ws2tcpip.h>	// For anything networking

#include <stdio.h>		// Logger.cpp:	For variadic function
#include <stdarg.h>		// Logger.cpp:	For variadic function
#include <time.h>		// Logger.cpp:	For Current Date/Time
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>		// Logger.cpp:	For string concatenation
#include <iostream>		// Logger.cpp:	For file writing
#include <fstream>		// Logger.cpp:	For file writing

#include "Logger.h"
#include "WSAManager.h"
#include "TCPStream.h"
#include "TCPClient.h"
#include "TCPServer.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

// TODO: reference additional headers your program requires here