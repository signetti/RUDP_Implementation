/*
*   C++ sockets on Unix and Windows
*   Copyright (C) 2002
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#ifndef WIN32
typedef int SOCKET;             // Socket descriptor
#endif

#pragma warning(disable : 4996)

#ifdef WIN32
#include <winsock.h>         // For socket(), connect(), send(), and recv()
typedef int socklen_t;
typedef char raw_type;       // Type used for raw data on this platform
#else
#include <sys/types.h>       // For data types
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
#include <netinet/in.h>      // For sockaddr_in
typedef void raw_type;       // Type used for raw data on this platform
#endif

#include <errno.h>             // For errno

#include "SocketException.h"
