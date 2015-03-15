////////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501

///////////////////////////////////////////////////////////////////////////////

// Windows headers
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <mmsystem.h>

// COM headers
#include <unknwn.h>

// Standard C headers
#include <sys/timeb.h>
#include <cstdio>
#include <ctime>
#include <cstdlib>

// STL headers
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stack>
#include <algorithm>
#include <list>
#include <map>

// DirectX 9.0 headers
#include <d3d9.h>
#include <d3dx9.h>

// Other headers
#include "detours.h"
#include "tinyxml.h"

////////////////////////////////////////////////////////////////////////////////
