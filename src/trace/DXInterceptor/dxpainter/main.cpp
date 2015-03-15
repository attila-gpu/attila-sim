////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXTraceManagerHeaders.h"
#include "GeometryTypes.h"
#include "DXPainter.h"
#include "main.h"

using namespace dxpainter;

////////////////////////////////////////////////////////////////////////////////

void my_c_exception_translator(unsigned code, EXCEPTION_POINTERS* ptrs)
{
  throw code;
}

////////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
    ::DisableThreadLibraryCalls(hinstDLL);
    _set_se_translator(my_c_exception_translator);
    return TRUE;
    break;

	case DLL_PROCESS_DETACH:
    return FALSE;
    break;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

EXPIMP_DLL DXPainter* WINAPI DXPainterCreate()
{
  return new DXPainter();
}

////////////////////////////////////////////////////////////////////////////////
