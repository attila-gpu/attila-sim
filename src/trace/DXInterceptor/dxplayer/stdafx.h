////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0501

// Algunos constructores CString serán explícitos
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

// Desactiva la ocultación de MFC para algunos mensajes de advertencia comunes y
// omitidos de forma consciente muchas veces.
#define _AFX_ALL_WARNINGS

////////////////////////////////////////////////////////////////////////////////

// MFC headers
#include <afxmt.h>
#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <afxpriv.h>
#include <afxtempl.h>

// Windows headers
#include <shlwapi.h>
#include <shlobj.h>
#include <unknwn.h>
#include <mmsystem.h>
#include <gdiplus.h>

// Standard C headers
#include <sys/timeb.h>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <process.h>

// STL headers
#include <fstream>
#include <sstream>
#include <iomanip>
#include <locale>
#include <stack>
#include <vector>
#include <list>
#include <map>

////////////////////////////////////////////////////////////////////////////////
