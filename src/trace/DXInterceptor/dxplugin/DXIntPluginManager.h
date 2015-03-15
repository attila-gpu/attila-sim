////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "DXIntPlugin.h"
#include "DXIntPluginLoaded.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxplugin
{
  class DXIntPluginManager
  {
  public:

    DXIntPluginManager();
    virtual ~DXIntPluginManager();

    void Clear();
    
    bool LoadAllPluginsFromDirectory(const std::string& directory = "");
    bool LoadPlugin(const std::string& filePath);
    bool LoadPlugin(const std::string& filePath, DXIntPluginLoaded** pluginLoaded);
        
    unsigned int GetPluginCount() const;
    bool GetPlugin(unsigned int position, DXIntPluginLoaded** plugin) const;
    
    unsigned int GetCounterToRecordCount() const;
    bool AddCounterToRecordList(const DXIntPluginLoaded* plugin, DXINTCOUNTERID counterID);
    
    bool BeginExperiment();
    bool ProcessCall(dxtraceman::DXMethodCallPtr call);
    bool EndExperiment();

    bool GetStatisticsLegend(dxtraceman::DXStatisticPtr stats);
    bool GetStatisticsFrame(dxtraceman::DXStatisticPtr stats);

  protected:

    typedef BOOL (*RegisterUnloadedPluginByExitProcessNotifierPtr)(PluginUnloadedByExitProcessNotifierPtr function);
    typedef BOOL (*GetPluginInfoPtr)(DXINTPLUGININFO* info);
    typedef BOOL (*GetCounterInfoPtr)(DWORD* countersCount, DXINTCOUNTERINFO** countersArray);
    typedef BOOL (*BeginExperimentPtr)(DXINTCOUNTERID counterID);
    typedef BOOL (*ProcessCallPtr)(DXINTCOUNTERID counterID, dxtraceman::DXMethodCallPtr call);
    typedef BOOL (*EndFramePtr)(DXINTCOUNTERID counterID, BYTE** data, UINT* size);
    typedef BOOL (*EndExperimentPtr)(DXINTCOUNTERID counterID);

    struct CounterProcessInformation
    {
      const DXIntPluginLoaded* Plugin;
      DXINTCOUNTERINFO Counter;
      
      BeginExperimentPtr BeginExperiment;
      ProcessCallPtr ProcessCall;
      EndFramePtr EndFrame;
      EndExperimentPtr EndExperiment;
      
      bool IsExperimentBegin;
      bool IsExperimentEnded;
    };
    
    std::vector<DXIntPluginLoaded*> m_plugins;
    std::vector<CounterProcessInformation> m_counters;

    bool FreePlugin(DXIntPluginLoaded* plugin);
    void PluginUnloaded(HMODULE pluginHandle);
    bool FindPlugin(HMODULE pluginHandle, DXIntPluginLoaded** plugin) const;
    static dxtraceman::DXStatistic::StatisticDataType ConvertType(DXINTCOUNTERDATATYPE type);

  };
}

////////////////////////////////////////////////////////////////////////////////
