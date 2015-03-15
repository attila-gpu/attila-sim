////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "resource.h"

////////////////////////////////////////////////////////////////////////////////
// Enable Direct3D secure multithread support.

#define D3D_MULTITHREAD_SUPPORT

#ifdef D3D_MULTITHREAD_SUPPORT
extern CRITICAL_SECTION gCriticalSection;
#endif // ifdef D3D_MULTITHREAD_SUPPORT

////////////////////////////////////////////////////////////////////////////////
// Save intercepted method calls names in LOG (to debug purposes only when a
// game crashes in the middle of a interception session).

//#define WRITE_METHOD_CALLS_NAMES_TO_LOG

////////////////////////////////////////////////////////////////////////////////

extern HMODULE gDllHandle;
extern dxtraceman::DXTraceManager* g_traceman;
extern dxplugin::DXIntPluginManager* g_statman;
extern DXLogger* g_logger;
extern DXInterceptorOptions* g_options;

extern "C" IDirect3D9* WINAPI D3DCreate9(UINT SDKVersion);

extern "C" int     WINAPI D3DPERF_BeginEvent(D3DCOLOR color, LPCWSTR name);
extern "C" int     WINAPI D3DPERF_EndEvent();
extern "C" void    WINAPI D3DPERF_SetOptions(DWORD options);
extern "C" DWORD   WINAPI D3DPERF_GetStatus(void);
extern "C" BOOL    WINAPI D3DPERF_QueryRepeatFrame(void);
extern "C" void    WINAPI D3DPERF_SetMarker(D3DCOLOR color, LPCWSTR name);
extern "C" void    WINAPI D3DPERF_SetRegion(D3DCOLOR color, LPCWSTR name);
extern "C" HRESULT WINAPI DebugSetMute(DWORD options);
extern "C" HRESULT WINAPI DebugSetLevel(void);

////////////////////////////////////////////////////////////////////////////////
