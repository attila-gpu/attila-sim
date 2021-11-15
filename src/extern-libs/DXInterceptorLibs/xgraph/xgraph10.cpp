#include "stdafx.h"
#include <afxdllx.h>

static AFX_EXTENSION_MODULE XGraphLibDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("XGRAPH.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(XGraphLibDLL, hInstance))
			return 0;
	
		new CDynLinkLibrary(XGraphLibDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("XGRAPH.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(XGraphLibDLL);
	}
	return 1;   // ok
}
