////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXTraceManagerHeaders.h"
#include "DXIntPluginManager.h"

using namespace std;
using namespace dxtraceman;
using namespace dxplugin;

////////////////////////////////////////////////////////////////////////////////

DXIntPluginManager::DXIntPluginManager()
{
}

////////////////////////////////////////////////////////////////////////////////

DXIntPluginManager::~DXIntPluginManager()
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::LoadAllPluginsFromDirectory(const std::string& directory)
{
  HANDLE filehandle;
  WIN32_FIND_DATA fileentry;
  char filepath[MAX_PATH];
  unsigned int numLoaded = 0;

  strcpy(filepath, directory.c_str());
  strcat(filepath, "*.dxintplugin");
  
  filehandle = FindFirstFile(filepath, &fileentry);

  if (filehandle == INVALID_HANDLE_VALUE)
  {
    return false;
  }

  do
  {
    if (strcmp(fileentry.cFileName, ".") == 0 || strcmp(fileentry.cFileName, "..") == 0)
    {
      continue;
    }
    if (LoadPlugin(directory + fileentry.cFileName))
    {
      numLoaded++;
    }
  } while (FindNextFile(filehandle, &fileentry) != 0);

  FindClose(filehandle);
  
  return (numLoaded > 0);
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::LoadPlugin(const string& filePath)
{
  return LoadPlugin(filePath, NULL);
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::LoadPlugin(const string& filePath, DXIntPluginLoaded** pluginLoaded)
{
  HMODULE hDLL = NULL;
  
  if (pluginLoaded != NULL)
  {
    *pluginLoaded = NULL;
  }
  
  hDLL = LoadLibrary(filePath.c_str());
  if (hDLL == NULL)
  {
    return false;
  }

  GetPluginInfoPtr GetPluginInfo = (GetPluginInfoPtr) GetProcAddress(hDLL, _T("DXIntPlugin_GetPluginInfo"));
  if (GetPluginInfo == NULL)
  {
    FreeLibrary(hDLL);
    return false;
  }
  
  DXINTPLUGININFO pluginfo;
  
  if (!GetPluginInfo(&pluginfo))
  {
    FreeLibrary(hDLL);
    return false;
  }

  if (pluginfo.SystemVersion != DXINT_PLUGIN_SYSTEM_VERSION)
  {
    FreeLibrary(hDLL);
    return false;
  }
  
  GetCounterInfoPtr GetCounterInfo = (GetCounterInfoPtr) GetProcAddress(hDLL, _T("DXIntPlugin_GetCounterInfo"));
  if (GetCounterInfo == NULL)
  {
    FreeLibrary(hDLL);
    return false;
  }
  
  DWORD countersCount;
  DXINTCOUNTERINFO* counters;
  if (!GetCounterInfo(&countersCount, &counters))
  {
    FreeLibrary(hDLL);
    return false;
  }

  TCHAR szDllPath[MAX_PATH];
  ::GetModuleFileName(hDLL, szDllPath, MAX_PATH);
  
  DXIntPluginLoaded* plugin = new DXIntPluginLoaded(szDllPath, hDLL, pluginfo.Version, pluginfo.Name);

  for (unsigned int i=0; i < countersCount; ++i)
  {
    plugin->AddCounter(counters[i]);
  }
    
  m_plugins.push_back(plugin);
  
  if (pluginLoaded != NULL)
  {
    *pluginLoaded = plugin;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::FreePlugin(DXIntPluginLoaded* plugin)
{
  if (plugin->GetDllHandle() != NULL)
  {
    FreeLibrary(plugin->GetDllHandle());
  }  
  delete plugin;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXIntPluginManager::Clear()
{
  for (vector<DXIntPluginLoaded*>::iterator it=m_plugins.begin(); it != m_plugins.end(); ++it)
  {
    FreePlugin(*it);
  }
  m_plugins.clear();
  m_counters.clear();
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXIntPluginManager::GetPluginCount() const
{
  return (unsigned int) m_plugins.size();
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::GetPlugin(unsigned int position, DXIntPluginLoaded** plugin) const
{
  if (!plugin)
  {
    return false;
  }

  if (position >= 0 && position < m_plugins.size())
  {
    *plugin = m_plugins[position];
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXIntPluginManager::GetCounterToRecordCount() const
{
  return (unsigned int) m_counters.size();
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::AddCounterToRecordList(const DXIntPluginLoaded* plugin, DXINTCOUNTERID counterID)
{
  DXINTCOUNTERINFO counter;
  if (!plugin->CounterFind(counterID, &counter))
  {
    return false;
  }
  
  CounterProcessInformation counterInfo;
  
  counterInfo.Plugin = plugin;
  counterInfo.Counter = counter;
  counterInfo.IsExperimentBegin = false;
  counterInfo.IsExperimentEnded = false;
  
  RegisterUnloadedPluginByExitProcessNotifierPtr registryNotifier = (RegisterUnloadedPluginByExitProcessNotifierPtr) GetProcAddress(plugin->GetDllHandle(), _T("DXIntPlugin_RegisterUnloadedPluginByExitProcessNotifier"));
  if (registryNotifier == NULL)
  {
    return false;
  }
  else
  {
    registryNotifier(fastdelegate::MakeDelegate(this, &DXIntPluginManager::PluginUnloaded));
  }
  
  counterInfo.BeginExperiment = (BeginExperimentPtr) GetProcAddress(plugin->GetDllHandle(), _T("DXIntPlugin_BeginExperiment"));
  if (counterInfo.BeginExperiment == NULL)
  {
    return false;
  }

  counterInfo.ProcessCall = (ProcessCallPtr) GetProcAddress(plugin->GetDllHandle(), _T("DXIntPlugin_ProcessCall"));
  if (counterInfo.ProcessCall == NULL)
  {
    return false;
  }

  counterInfo.EndFrame = (EndFramePtr) GetProcAddress(plugin->GetDllHandle(), _T("DXIntPlugin_EndFrame"));
  if (counterInfo.EndFrame == NULL)
  {
    return false;
  }

  counterInfo.EndExperiment = (EndExperimentPtr) GetProcAddress(plugin->GetDllHandle(), _T("DXIntPlugin_EndExperiment"));
  if (counterInfo.EndExperiment == NULL)
  {
    return false;
  }
  
  m_counters.push_back(counterInfo);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXIntPluginManager::PluginUnloaded(HMODULE pluginHandle)
{
  // end the experiment and delete the counters from this plugin
  
  for (vector<CounterProcessInformation>::iterator it=m_counters.begin(); it != m_counters.end();)
  {
    if ((*it).Plugin->GetDllHandle() == pluginHandle)
    {
      if ((*it).IsExperimentBegin && !(*it).IsExperimentEnded)
      {
        (*it).EndExperiment((*it).Counter.ID);
        (*it).IsExperimentEnded = true;
      }
      it = m_counters.erase(it);
    }
    else
    {
      it++;
    }
  }

  // delete references to this plugin
  
  for (vector<DXIntPluginLoaded*>::iterator it=m_plugins.begin(); it != m_plugins.end(); ++it)
  {
    if ((*it)->GetDllHandle() == pluginHandle)
    {
      delete *it;
      it = m_plugins.erase(it);
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::FindPlugin(HMODULE pluginHandle, DXIntPluginLoaded** plugin) const
{
  bool found = false;
  *plugin = NULL;  
  
  for (vector<DXIntPluginLoaded*>::const_iterator it=m_plugins.begin(); it != m_plugins.end(); ++it)
  {
    if ((*it)->GetDllHandle() == pluginHandle)
    {
      *plugin = *it;
      found = true;
      break;
    }
  }

  return found;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::BeginExperiment()
{
  if (!m_counters.size())
  {
    return false;
  }
  
  for (vector<CounterProcessInformation>::iterator it=m_counters.begin(); it != m_counters.end(); ++it)
  {
    if ((*it).BeginExperiment((*it).Counter.ID))
    {
      (*it).IsExperimentBegin = true;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::ProcessCall(DXMethodCallPtr call)
{
  if (!m_counters.size())
  {
    return false;
  }
  
  for (vector<CounterProcessInformation>::iterator it=m_counters.begin(); it != m_counters.end(); ++it)
  {
    (*it).ProcessCall((*it).Counter.ID, call);
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::EndExperiment()
{
  if (!m_counters.size())
  {
    return false;
  }
  
  for (vector<CounterProcessInformation>::iterator it=m_counters.begin(); it != m_counters.end(); ++it)
  {
    if ((*it).IsExperimentBegin && !(*it).IsExperimentEnded)
    {
      (*it).EndExperiment((*it).Counter.ID);
      (*it).IsExperimentEnded = true;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::GetStatisticsLegend(DXStatisticPtr stats)
{
  if (!m_counters.size())
  {
    return false;
  }
  
  stats->Clear();
  stats->SetType(DXStatistic::ST_LEGEND);
  
  for (vector<CounterProcessInformation>::iterator it=m_counters.begin(); it != m_counters.end(); ++it)
  {
    stats->AddLegend((*it).Counter.Name, (*it).Counter.Description, ConvertType((*it).Counter.DataType));
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginManager::GetStatisticsFrame(DXStatisticPtr stats)
{
  if (!m_counters.size())
  {
    return false;
  }
  
  stats->Clear();
  stats->SetType(DXStatistic::ST_COUNTER);
  stats->SetSyncPaquet(0); // PERFER : falta proporcionar paquet de sincronia!!!
  
  BYTE* data = NULL;
  UINT size  = 0;
  for (vector<CounterProcessInformation>::iterator it=m_counters.begin(); it != m_counters.end(); ++it)
  {
    if ((*it).EndFrame((*it).Counter.ID, &data, &size) && data != NULL)
    {
      switch ((*it).Counter.DataType)
      {
      case DXICDT_UINT32:
        stats->AddCounterData(*((dx_uint32*) data));
        break;
      case DXICDT_UINT64:
        stats->AddCounterData(*((dx_uint64*) data));
        break;
      case DXICDT_FLOAT:
        stats->AddCounterData(*((dx_float*) data));
        break;
      case DXICDT_DOUBLE:
        stats->AddCounterData(*((dx_double*) data));
        break;
      case DXICDT_STRING:
        stats->AddCounterData((char*) data);
        break;
      }
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

DXStatistic::StatisticDataType DXIntPluginManager::ConvertType(DXINTCOUNTERDATATYPE type)
{
  switch (type)
  {
  default:
  case DXICDT_UINT32:
    return DXStatistic::SDT_UINT32;
  case DXICDT_UINT64:
    return DXStatistic::SDT_UINT64;
  case DXICDT_FLOAT:
    return DXStatistic::SDT_FLOAT;
  case DXICDT_DOUBLE:
    return DXStatistic::SDT_DOUBLE;
  case DXICDT_STRING:
    return DXStatistic::SDT_STRING;
  }
}

////////////////////////////////////////////////////////////////////////////////
