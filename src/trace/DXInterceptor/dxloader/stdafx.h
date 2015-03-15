////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0501

// Algunos constructores CString serán explícitos
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

// Desactiva la ocultación de MFC para algunos mensajes de advertencia comunes
// y, muchas veces, omitidos de forma consciente.
#define _AFX_ALL_WARNINGS

////////////////////////////////////////////////////////////////////////////////

// MFC headers
#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <afxpriv.h>
#include <afxtempl.h>
#include <afxdtctl.h>

// Windows headers
#include <shlwapi.h>
#include <shlobj.h>
#include <winver.h>

// Standard C headers
#include <sys/timeb.h>
#include <cstdio>
#include <ctime>
#include <cstdlib>

// STL headers
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <exception>
#include <map>

// DirectX 9.0 headers
#include <d3d9.h>

////////////////////////////////////////////////////////////////////////////////

#include "tinyxml.h"
#include "detours.h"

////////////////////////////////////////////////////////////////////////////////
