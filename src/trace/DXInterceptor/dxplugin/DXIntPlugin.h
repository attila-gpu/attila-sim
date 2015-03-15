////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FastDelegate.h"

////////////////////////////////////////////////////////////////////////////////

#define DLLEXP __declspec(dllexport)

////////////////////////////////////////////////////////////////////////////////
// DXINT_PLUGIN_SYSTEM_VERSION
//
// Indicates version of the plugin interface the plugin is built with.
////////////////////////////////////////////////////////////////////////////////

#define DXINT_PLUGIN_SYSTEM_VERSION 0x100

////////////////////////////////////////////////////////////////////////////////
// DXINTCOUNTERID
//
// A unique identifier for each DXInterceptor plugin counter.
////////////////////////////////////////////////////////////////////////////////

typedef unsigned int DXINTCOUNTERID;

////////////////////////////////////////////////////////////////////////////////
// DXINTCOUNTERDATATYPE
//
// Indicates what type of data the counter produces.
////////////////////////////////////////////////////////////////////////////////

enum DXINTCOUNTERDATATYPE
{
  DXICDT_UINT32 = 1,
  DXICDT_UINT64 = 2,
  DXICDT_FLOAT  = 3,
  DXICDT_DOUBLE = 4,
  DXICDT_STRING = 5,
};

////////////////////////////////////////////////////////////////////////////////
// DXINTPLUGININFO
//
// This structure is filled out by DXIntPlugin_GetPluginInfo and passed back to
// DXInterceptor plugin manager.
////////////////////////////////////////////////////////////////////////////////

struct DXINTPLUGININFO
{
  char*        Name;          // Name of the plugin
  unsigned int Version;       // Version of this particular plugin
  unsigned int SystemVersion; // Version of DXInterceptor's plugin system this plugin was designed for
};

////////////////////////////////////////////////////////////////////////////////
// DXINTCOUNTERINFO
//
// This structure is filled out by DXIntPlugin_GetCounterInfo and passed back to
// DXInterceptor plugin manager to determine information about the counters in
// the plugin.
////////////////////////////////////////////////////////////////////////////////

struct DXINTCOUNTERINFO
{
  DXINTCOUNTERID       ID;          // Used to uniquely ID this counter
  char*                Name;        // Name of the counter
  char*                Description; // Description of the counter
  DXINTCOUNTERDATATYPE DataType;    // Data type returned by this counter
};

////////////////////////////////////////////////////////////////////////////////
// DXIntPlugin_RegisterUnloadedPluginByExitProcessNotifier
//
// This register a callback to a function that is called when the plugin is
// unloaded by ExitProcess and not by FreeLibrary. Occurs when the plugin is
// freed by de OS and not by the application.
////////////////////////////////////////////////////////////////////////////////

typedef fastdelegate::FastDelegate1<HMODULE> PluginUnloadedByExitProcessNotifierPtr;

extern "C" DLLEXP BOOL DXIntPlugin_RegisterUnloadedPluginByExitProcessNotifier(PluginUnloadedByExitProcessNotifierPtr function);

////////////////////////////////////////////////////////////////////////////////
// Define the above constant to include in your plugin code a default DLL load
// manager that manages correctly the plugin unload situation.

#ifdef DXINTPLUGIN_IMPLEMENTATION

PluginUnloadedByExitProcessNotifierPtr DXIntPlugin_notifyPluginUnload;

extern "C" DLLEXP BOOL DXIntPlugin_RegisterUnloadedPluginByExitProcessNotifier(PluginUnloadedByExitProcessNotifierPtr function)
{
  DXIntPlugin_notifyPluginUnload = function;
  return TRUE;
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
    ::DisableThreadLibraryCalls(hinstDLL);
    return TRUE;
  case DLL_PROCESS_DETACH:
    if (lpReserved && DXIntPlugin_notifyPluginUnload)
    {
      DXIntPlugin_notifyPluginUnload(hinstDLL);
      DXIntPlugin_notifyPluginUnload = NULL;
    }
    return TRUE;
  }
  return FALSE;
}

#endif // DXINTPLUGIN_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////////////
// DXIntPlugin_GetPluginInfo
//
// This returns basic information about this plugin to DXInterceptor.
////////////////////////////////////////////////////////////////////////////////

extern "C" DLLEXP BOOL DXIntPlugin_GetPluginInfo(DXINTPLUGININFO* info);

////////////////////////////////////////////////////////////////////////////////
// DXIntPlugin_GetCounterInfo
//
// This returns an array of DXINTCOUNTERINFO structs to DXInterceptor plugin
// manager. These DXINTCOUNTERINFOs allow DXInterceptor plugin manager to
// enumerate the counters contained in this plugin.
////////////////////////////////////////////////////////////////////////////////

extern "C" DLLEXP BOOL DXIntPlugin_GetCounterInfo(DWORD* countersCount, DXINTCOUNTERINFO** countersArray);

////////////////////////////////////////////////////////////////////////////////
// DXIntPlugin_BeginExperiment
//
// This called by DXInterceptor once per counter when instrumentation starts.
////////////////////////////////////////////////////////////////////////////////

extern "C" DLLEXP BOOL DXIntPlugin_BeginExperiment(DXINTCOUNTERID counterID);

////////////////////////////////////////////////////////////////////////////////
// DXIntPlugin_ProcessCall
//
// This called by DXInterceptor once per call before be saved to disc.
////////////////////////////////////////////////////////////////////////////////

extern "C" DLLEXP BOOL DXIntPlugin_ProcessCall(DXINTCOUNTERID counterID, dxtraceman::DXMethodCallPtr call);

////////////////////////////////////////////////////////////////////////////////
// DXIntPlugin_EndFrame
//
// This is called by DXInterceptor once per counter at the end of each frame to
// gather the counter value for that frame. Note that the pointer to the return
// data must continue to point to valid counter data until the next call to
// DXIntPlugin_EndFrame (or DXIntPlugin_EndExperiment) for the same counter. So
// do not set *ppReturnData to the same pointer for multiple counters, or point
// to a local variable that will go out of scope.
////////////////////////////////////////////////////////////////////////////////

extern "C" DLLEXP BOOL DXIntPlugin_EndFrame(DXINTCOUNTERID counterID, BYTE** data, UINT* size);

////////////////////////////////////////////////////////////////////////////////
// DXIntPlugin_EndExperiment
//
// This is called by DXInterceptor once per counter when instrumentation ends.
////////////////////////////////////////////////////////////////////////////////

extern "C" DLLEXP BOOL DXIntPlugin_EndExperiment(DXINTCOUNTERID counterID);

////////////////////////////////////////////////////////////////////////////////
