////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXTraceManagerHeaders.h"
#include "DXIntPluginLoaded.h"

using namespace std;
using namespace dxplugin;

////////////////////////////////////////////////////////////////////////////////

DXIntPluginLoaded::DXIntPluginLoaded(const string& filename, HMODULE dllHandle, unsigned int version, const string& name) :
m_filename(filename),
m_dllHandle(dllHandle),
m_version(version),
m_name(name)
{
}

////////////////////////////////////////////////////////////////////////////////

DXIntPluginLoaded::DXIntPluginLoaded(const DXIntPluginLoaded& plugin)
{
  CloneObjects(plugin, *this);
}

////////////////////////////////////////////////////////////////////////////////

DXIntPluginLoaded::~DXIntPluginLoaded()
{
}

////////////////////////////////////////////////////////////////////////////////

DXIntPluginLoaded& DXIntPluginLoaded::operator = (const DXIntPluginLoaded& plugin)
{
  CloneObjects(plugin, *this);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

const string& DXIntPluginLoaded::GetFileName() const
{
  return m_filename;
}

////////////////////////////////////////////////////////////////////////////////

HMODULE DXIntPluginLoaded::GetDllHandle() const
{
  return m_dllHandle;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXIntPluginLoaded::GetVersion() const
{
  return m_version;
}

////////////////////////////////////////////////////////////////////////////////

const string& DXIntPluginLoaded::GetName() const
{
  return m_name;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXIntPluginLoaded::GetCounterCount() const
{
  return (unsigned int) m_counters.size();
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginLoaded::GetCounter(unsigned int position, DXINTCOUNTERINFO* counter) const
{
  if (!counter)
  {
    return false;
  }
  
  if (position >= 0 && position < m_counters.size())
  {
    *counter = m_counters[position];
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXIntPluginLoaded::AddCounter(DXINTCOUNTERINFO counter)
{
  unsigned int counterNumber = (unsigned int) m_counters.size();
  m_counters.push_back(counter);
  return counterNumber;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginLoaded::CounterExists(DXINTCOUNTERID counterID) const
{
  for (unsigned int i=0; i < (unsigned int) m_counters.size(); ++i)
  {
    if (m_counters[i].ID == counterID)
    {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIntPluginLoaded::CounterFind(DXINTCOUNTERID counterID, DXINTCOUNTERINFO* counter) const
{
  for (unsigned int i=0; i < (unsigned int) m_counters.size(); ++i)
  {
    if (m_counters[i].ID == counterID)
    {
      *counter = m_counters[i];
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void DXIntPluginLoaded::CloneObjects(const DXIntPluginLoaded& orig, DXIntPluginLoaded& dest)
{
  dest.m_filename = orig.m_filename;
  dest.m_dllHandle = orig.m_dllHandle;
  dest.m_version = orig.m_version;
  dest.m_name = orig.m_name;

  dest.m_counters.clear();
  copy(orig.m_counters.begin(), orig.m_counters.end(), back_inserter(dest.m_counters));
}

////////////////////////////////////////////////////////////////////////////////
