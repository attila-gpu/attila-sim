////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXLogger.h"
#include "DXInterceptorOptions.h"
#include "IDirect3D9InterceptorWrapper.h"
#include "DXTraceManagerHeaders.h"
#include "DXIntPluginManager.h"
#include "main.h"

using namespace std;
using namespace dxtraceman;
using namespace dxplugin;

////////////////////////////////////////////////////////////////////////////////
// Compilation flags
////////////////////////////////////////////////////////////////////////////////

// Enable Detours in the DLL
#define USE_DETOURS

// Enable interception of additional methods from d3d9.dll
//#define LOG_ADDITIONAL_METHODS

////////////////////////////////////////////////////////////////////////////////

HMODULE gDllHandle = NULL;

#ifdef D3D_MULTITHREAD_SUPPORT
CRITICAL_SECTION gCriticalSection;
#endif // ifdef D3D_MULTITHREAD_SUPPORT

#ifndef USE_DETOURS
HMODULE lD3DHandle = NULL;
#endif // USE_DETOURS

IDirect3D9* (WINAPI* s_pReal_Direct3DCreate9)(UINT) = NULL;

DXTraceManager* g_traceman = NULL;
DXIntPluginManager* g_statman = NULL;
DXLogger* g_logger = NULL;
DXInterceptorOptions* g_options = NULL;
int D3DPERF_EventLevel = 0;

////////////////////////////////////////////////////////////////////////////////

bool SetupDirect3D9();
bool UnSetupDirect3D9();

bool SetupTraceManager();
void UnSetupTraceManager();

void SetupLogger();
void UnSetupLogger();

#ifdef D3D_MULTITHREAD_SUPPORT
void SetupMultithread();
void UnSetupMultithread();
#endif // ifdef D3D_MULTITHREAD_SUPPORT

////////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    fstream logFile;
    
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
    
    if (!gDllHandle)
    {
      ::DisableThreadLibraryCalls(hinstDLL);

#ifdef USE_DETOURS
      DetourRestoreAfterWith();
#endif // USE_DETOURS

#if (_WIN32_WINNT >= 0x0501)
      ::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPTSTR) DllMain, &gDllHandle);
#else
      {
        TCHAR szDllPath[MAX_PATH];
        ::GetModuleFileName((HMODULE) hinstDLL, szDllPath, MAX_PATH);
        gDllHandle = ::LoadLibrary(szDllPath);
      }
#endif
      
      if (!gDllHandle)
      {
        return FALSE;
      }
   
#ifdef D3D_MULTITHREAD_SUPPORT
      SetupMultithread();
#endif // ifdef D3D_MULTITHREAD_SUPPORT
      
      if (SetupDirect3D9())
      {
        return TRUE;
      }
    }
    
    break;

	case DLL_PROCESS_DETACH:
    
    UnSetupTraceManager();
    UnSetupDirect3D9();
    UnSetupLogger();

#ifdef D3D_MULTITHREAD_SUPPORT
    UnSetupMultithread();
#endif // ifdef D3D_MULTITHREAD_SUPPORT

    gDllHandle = NULL;

    break;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

bool SetupDirect3D9()
{
	if (s_pReal_Direct3DCreate9)
  {
    return true;
  }
	
	TCHAR pathD3D[MAX_PATH];
  ::GetSystemDirectory(pathD3D, MAX_PATH);
  ::PathAddBackslash(pathD3D);
  ::PathAppend(pathD3D, TEXT("d3d9.dll"));
   
#ifdef USE_DETOURS

  s_pReal_Direct3DCreate9 = (IDirect3D9* (WINAPI *)(UINT)) DetourFindFunction(pathD3D, "Direct3DCreate9");
  if (s_pReal_Direct3DCreate9)
  {
    if (DetourTransactionBegin()) return false;
    if (DetourUpdateThread(GetCurrentThread())) return false;
    if (DetourAttach(&(PVOID&) s_pReal_Direct3DCreate9, D3DCreate9)) return false;
    return (DetourTransactionCommit() == NO_ERROR);
  }

#else

  lD3DHandle = ::LoadLibrary(pathD3D);
  if (lD3DHandle)
  {
    s_pReal_Direct3DCreate9 = (IDirect3D9* (WINAPI*)(UINT)) GetProcAddress(lD3DHandle, "Direct3DCreate9");
    return (s_pReal_Direct3DCreate9 != NULL);
  }

#endif // USE_DETOURS

  return false;	
}

////////////////////////////////////////////////////////////////////////////////

bool UnSetupDirect3D9()
{
  if (s_pReal_Direct3DCreate9)
  {
#ifdef USE_DETOURS

    if (DetourTransactionBegin()) return false;
    if (DetourUpdateThread(GetCurrentThread())) return false;
    if (DetourDetach(&(PVOID&) s_pReal_Direct3DCreate9, D3DCreate9)) return false;
    return (DetourTransactionCommit() == NO_ERROR);

#else

    if (lD3DHandle)
    {
      ::FreeLibrary(lD3DHandle);
      lD3DHandle = NULL;
      return true;
    }

#endif // USE_DETOURS
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool SetupTraceManager()
{  
  TCHAR szDllPath[MAX_PATH];
  TCHAR szConfigFilename[MAX_PATH];
  TCHAR szExecutableName[MAX_PATH];
  
  ::GetModuleFileName(gDllHandle, szDllPath, MAX_PATH);
  
  _tcsncpy(szConfigFilename, szDllPath, MAX_PATH);
  ::PathRenameExtension(szConfigFilename, TEXT(".config"));
  
  ::PathRemoveFileSpec(szDllPath);
  ::PathAddBackslash(szDllPath);
  
  ::GetModuleFileName(NULL, szExecutableName, MAX_PATH);
  ::PathStripPath(szExecutableName);

  //////////////////////////////////////////////////////////////////////////////
  // Load Options
  
  g_options = new DXInterceptorOptions();
  if (!g_options->LoadXML(szConfigFilename))
  {
    g_logger->Write("ERROR: Can't open configuration file '%s'.", szConfigFilename);
    g_options->Clear();
    g_options->SetDestinationPath(szDllPath);
  }
  
  if (!::PathFileExists(g_options->GetDestinationPath().c_str()))
  {
    g_logger->Write("WARNING: Traces destination path '%s' not found.", g_options->GetDestinationPath().c_str());
    g_logger->Write("         Changed traces destination path to '%s'.", szDllPath);
    g_options->SetDestinationPath(szDllPath);
  }  
  
  //////////////////////////////////////////////////////////////////////////////
  // Load Statistics Plugins

  if (g_options->GetPluginCount())
  {
    g_statman = new DXIntPluginManager();

    for (unsigned int i=0; i < g_options->GetPluginCount(); ++i)
    {
      DXInterceptorOptions::StatisticsPlugin plugin;
      if (g_options->GetPlugin(i, plugin))
      {
        DXIntPluginLoaded* pluginLoaded;
        if (g_statman->LoadPlugin(plugin.PluginFileName, &pluginLoaded))
        {
          g_logger->Write("INFO: Statistics Plugin loaded '%s'", pluginLoaded->GetFileName().c_str());
          for (unsigned int j=0; j < (unsigned int) plugin.Counters.size(); ++j)
          {
            if (g_statman->AddCounterToRecordList(pluginLoaded, plugin.Counters[j]))
            {
              DXINTCOUNTERINFO counterInfo;
              if (pluginLoaded->CounterFind(plugin.Counters[j], &counterInfo))
              {
                g_logger->Write("      Registered Counter [%u] '%s'", counterInfo.ID, counterInfo.Name);
              }
            }
          }
        }
      }
    }

    g_statman->BeginExperiment();
  }

  //////////////////////////////////////////////////////////////////////////////
  
  g_traceman = new DXTraceManager();
  if (g_traceman && g_traceman->OpenWrite(g_options->GetDestinationPath(), szExecutableName, g_options->GetCompression()))
  {
    ////////////////////////////////////////////////////////////////////////////
    // Save the statistics counters legend

    if (g_statman && g_statman->GetCounterToRecordCount())
    {
      DXStatisticPtr stats = new DXStatistic();
      if (g_statman->GetStatisticsLegend(stats))
      {
        unsigned int statistic_id = 0;
        g_traceman->WriteStatistic(stats, statistic_id);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Log trace options applied

    static char cadena[512];
    std::istringstream str(g_traceman->GetOptionsString());
    while (str.getline(cadena, 512))
    {
      g_logger->Write("%s", cadena);
    }

    ////////////////////////////////////////////////////////////////////////////
    
    return true;
  }
  else
  {
    g_logger->Write("ERROR: Can't create project file for '%s'.", szExecutableName);
    if (g_statman)
    {
      delete g_statman;
      g_statman = NULL;
    }
    if (g_traceman)
    {
      delete g_traceman;
      g_traceman = NULL;
    }
    if (g_options)
    {
      delete g_options;
      g_options = NULL;
    }
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

void UnSetupTraceManager()
{
  if (g_statman)
  {
    g_statman->EndExperiment();
    delete g_statman;
    g_statman = NULL;
  }
  
  if (g_options)
  {
    delete g_options;
    g_options = NULL;
  }
  
  if (g_traceman)
  {
    static char cadena[512];
    std::istringstream str(g_traceman->GetCountersString());
    while (str.getline(cadena, 512))
    {
      g_logger->Write("%s", cadena);
    }
    
    delete g_traceman;
    g_traceman = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

void SetupLogger()
{
  TCHAR szModuleName[MAX_PATH];
  TCHAR szLoggerName[MAX_PATH];
  ::GetModuleFileName(gDllHandle, szModuleName, MAX_PATH);
  _tcsncpy(szLoggerName, szModuleName, MAX_PATH);
  ::PathRenameExtension(szLoggerName, TEXT(".log"));
  
  g_logger = new DXLogger(szLoggerName);

  if (g_logger)
  {
    g_logger->Write("--------------------------------------------------------");
    g_logger->Write("BEGIN");
    g_logger->Write("INFO: Writed by '%s'", szModuleName);

#ifdef USE_DETOURS
    g_logger->Write("INFO: DLL compiled with Detours Library Support (dynamic binding allowed)");
#else
    g_logger->Write("INFO: DLL compiled without Detours Library Support (only static binding allowed)");
#endif // USE_DETOURS

#ifdef D3D_MULTITHREAD_SUPPORT
    g_logger->Write("INFO: DLL compiled with Multithreaded Direct3D Support");
#endif // D3D_MULTITHREAD_SUPPORT
  }
}

////////////////////////////////////////////////////////////////////////////////

void UnSetupLogger()
{
  if (g_logger)
  {
    g_logger->Write("END");
    g_logger->Write("--------------------------------------------------------");
    
    delete g_logger;
    g_logger = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

#ifdef D3D_MULTITHREAD_SUPPORT
void SetupMultithread()
{
  InitializeCriticalSection(&gCriticalSection);
}
#endif // ifdef D3D_MULTITHREAD_SUPPORT

////////////////////////////////////////////////////////////////////////////////

#ifdef D3D_MULTITHREAD_SUPPORT
void UnSetupMultithread()
{
  DeleteCriticalSection(&gCriticalSection);
}
#endif // ifdef D3D_MULTITHREAD_SUPPORT

////////////////////////////////////////////////////////////////////////////////

IDirect3D9* WINAPI D3DCreate9(UINT SDKVersion)
{
	if (!s_pReal_Direct3DCreate9)
  {
    return NULL;
  }

  if (!g_logger)
  {
    SetupLogger();
  }

  if (!g_traceman)
  {
    SetupTraceManager();
  }
  
	LPDIRECT3D9 lpD3DCreate9 = s_pReal_Direct3DCreate9(SDKVersion);
	if (!lpD3DCreate9)
	{
		g_logger->Write("ERROR: Failed to create Direct3D object");
		return NULL;
	}
	
  if (g_logger && g_traceman)
  {
    LPDIRECT3D9 lpInterceptor = new IDirect3D9InterceptorWrapper(IID_IDirect3D9, lpD3DCreate9);
    if (!lpInterceptor) 
    {
      g_logger->Write("ERROR: Failed to create IDirect3D9Interceptor object");
      return lpD3DCreate9;
    }
    else
    {
      DXMethodCallPtr call = new DXMethodCall(DXMethodCallHelper::TOK_Direct3DCreate9);
      call->SetCreatorID(0);
      call->Push_DWORD(SDKVersion);
      call->Push_DXResourceObjectID(((IDirect3D9InterceptorWrapper*) lpInterceptor)->GetObjectID());
      call->SetIsSavedReturnValue(true);
      g_traceman->WriteMethodCall(call);

      return lpInterceptor;
    }
  }
  else
  {
    return lpD3DCreate9;
  }
}

////////////////////////////////////////////////////////////////////////////////

int WINAPI D3DPERF_BeginEvent(D3DCOLOR color, LPCWSTR name)
{
#ifdef LOG_ADDITIONAL_METHODS
  if (g_logger)
  {
    g_logger->Write("D3DPERF_BeginEvent(%X, \"%s\") = %u", color, name, D3DPERF_EventLevel);
  }
#endif // ifdef LOG_ADDITIONAL_METHODS
  return D3DPERF_EventLevel++;
}

////////////////////////////////////////////////////////////////////////////////

int WINAPI D3DPERF_EndEvent(void)
{
#ifdef LOG_ADDITIONAL_METHODS
  if (g_logger)
  {
    g_logger->Write("D3DPERF_EndEvent() = %u", D3DPERF_EventLevel);
  }
#endif // ifdef LOG_ADDITIONAL_METHODS
  return --D3DPERF_EventLevel;
}

////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI D3DPERF_QueryRepeatFrame(void)
{
#ifdef LOG_ADDITIONAL_METHODS
  if (g_logger)
  {
    g_logger->Write("D3DPERF_QueryRepeatFrame()");
  }
#endif // ifdef LOG_ADDITIONAL_METHODS
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void WINAPI D3DPERF_SetMarker(D3DCOLOR color, LPCWSTR name)
{
#ifdef LOG_ADDITIONAL_METHODS
  if (g_logger)
  {
    g_logger->Write("D3DPERF_SetMarker(%X, \"%s\")", color, name);
  }
#endif // ifdef LOG_ADDITIONAL_METHODS
}

////////////////////////////////////////////////////////////////////////////////

void WINAPI D3DPERF_SetRegion(D3DCOLOR color, LPCWSTR name)
{
#ifdef LOG_ADDITIONAL_METHODS
  if (g_logger)
  {
    g_logger->Write("D3DPERF_SetRegion(%X, \"%s\")", color, name);
  }
#endif // ifdef LOG_ADDITIONAL_METHODS
}

////////////////////////////////////////////////////////////////////////////////

void WINAPI D3DPERF_SetOptions(DWORD options)
{
#ifdef LOG_ADDITIONAL_METHODS
  if (g_logger)
  {
    g_logger->Write("D3DPERF_SetOptions(%X)", options);
  }
#endif // ifdef LOG_ADDITIONAL_METHODS
}

////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI D3DPERF_GetStatus(void)
{
#ifdef LOG_ADDITIONAL_METHODS
  if (g_logger)
  {
    g_logger->Write("D3DPERF_GetStatus()");
  }
#endif // ifdef LOG_ADDITIONAL_METHODS
  return 1;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT WINAPI DebugSetMute(DWORD options)
{
#ifdef LOG_ADDITIONAL_METHODS
  if (g_logger)
  {
    g_logger->Write("DebugSetMute(%X)", options);
  }
#endif // ifdef LOG_ADDITIONAL_METHODS
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT WINAPI DebugSetLevel(void)
{
#ifdef LOG_ADDITIONAL_METHODS
  if (g_logger)
  {
    g_logger->Write("DebugSetLevel()");
  }
#endif // ifdef LOG_ADDITIONAL_METHODS
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
