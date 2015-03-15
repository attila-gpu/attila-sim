////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "DXIntPlugin.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxplugin
{
  class DXIntPluginLoaded
  {
  public:
    
    DXIntPluginLoaded(const std::string& filename, HMODULE dllHandle, unsigned int version, const std::string& name);
    DXIntPluginLoaded(const DXIntPluginLoaded& plugin);
    virtual ~DXIntPluginLoaded();

    DXIntPluginLoaded& operator = (const DXIntPluginLoaded& plugin);
    
    const std::string& GetFileName() const;
    HMODULE GetDllHandle() const;
    unsigned int GetVersion() const;
    const std::string& GetName() const;

    unsigned int GetCounterCount() const;
    bool GetCounter(unsigned int position, DXINTCOUNTERINFO* counter) const;
    unsigned int AddCounter(DXINTCOUNTERINFO counter);
    bool CounterExists(DXINTCOUNTERID counterID) const;
    bool CounterFind(DXINTCOUNTERID counterID, DXINTCOUNTERINFO* counter) const;

  protected:

    std::string m_filename;
    HMODULE m_dllHandle;
    unsigned int m_version;
    std::string m_name;

    std::vector<DXINTCOUNTERINFO> m_counters;

    static void CloneObjects(const DXIntPluginLoaded& orig, DXIntPluginLoaded& dest);

  };
}

////////////////////////////////////////////////////////////////////////////////
