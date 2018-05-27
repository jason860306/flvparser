// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WIN32
#include "targetver.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <list>
#include <functional>
#include <algorithm>
#ifdef _WIN32
#include <tchar.h>
#include <WinSock2.h>
#pragma warning(disable: 4996)
#pragma comment(lib, "Ws2_32.lib")
#endif

#ifndef _WIN32
#define _MAX_PATH	260
#define _MAX_FNAME	256
#define _MAX_EXT	256
#endif

// TODO: reference additional headers your program requires here
