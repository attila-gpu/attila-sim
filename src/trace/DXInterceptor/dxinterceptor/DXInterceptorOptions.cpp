////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <shlwapi.h>
#include <d3d9.h>
#include "regexpr2.h"
#include "tinyxml.h"
#include "XMLConfig.h"
#include "DXInterceptorOptions.h"

using namespace std;
using namespace regex;

////////////////////////////////////////////////////////////////////////////////

DXInterceptorOptions::DXInterceptorOptions()
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

DXInterceptorOptions::DXInterceptorOptions(const DXInterceptorOptions& options)
{
  CloneObjects(options, *this);
}

////////////////////////////////////////////////////////////////////////////////

DXInterceptorOptions::~DXInterceptorOptions()
{
}

////////////////////////////////////////////////////////////////////////////////

DXInterceptorOptions& DXInterceptorOptions::operator = (const DXInterceptorOptions& options)
{
  CloneObjects(options, *this);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorOptions::Clear()
{
  m_destinationPath = "";
  m_compression = false;
  m_bannerShow = true;
  m_bannerPosition = BP_TopLeft;
  m_bannerTextColor = D3DCOLOR_RGBA(255, 255, 255, 255);
}

////////////////////////////////////////////////////////////////////////////////

string DXInterceptorOptions::GetDestinationPath() const
{
  return m_destinationPath;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorOptions::SetDestinationPath(const string& value)
{
  m_destinationPath = value;
}

////////////////////////////////////////////////////////////////////////////////

bool DXInterceptorOptions::GetCompression() const
{
  return m_compression;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorOptions::SetCompression(bool value)
{
  m_compression = value;
}

////////////////////////////////////////////////////////////////////////////////

bool DXInterceptorOptions::GetBannerShow() const
{
  return m_bannerShow;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorOptions::SetBannerShow(bool value)
{
  m_bannerShow = value;
}

////////////////////////////////////////////////////////////////////////////////

DXInterceptorOptions::BannerPosition DXInterceptorOptions::GetBannerPosition() const
{
  return m_bannerPosition;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorOptions::SetBannerPosition(BannerPosition value)
{
  m_bannerPosition = value;
}

////////////////////////////////////////////////////////////////////////////////

D3DCOLOR DXInterceptorOptions::GetBannerTextColor() const
{
  return m_bannerTextColor;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorOptions::SetBannerTextColor(D3DCOLOR value)
{
  m_bannerTextColor = value;
}

////////////////////////////////////////////////////////////////////////////////

bool DXInterceptorOptions::LoadXML(const std::string& filename)
{
  if (::PathFileExists(filename.c_str()))
  {
    Clear();
    
    string cadena;
    XMLConfig xml(filename, true);

    if (xml.GetSectionText("configuration/destinationdir", cadena))
    {
      m_destinationPath = cadena;
    }

    if (xml.GetSectionAttribute("configuration/storage", "compression", cadena))
    {
      if (cadena.compare("true") == 0)
        m_compression = true;
      else
        m_compression = false;
    }

    if (xml.GetSectionAttribute("configuration/progressbanner", "show", cadena))
    {
      if (cadena.compare("true") == 0)
        m_bannerShow = true;
      else
        m_bannerShow = false;
    }

    if (xml.GetSectionAttribute("configuration/progressbanner", "position", cadena))
    {
      if (cadena.compare("top-left") == 0)
        m_bannerPosition = BP_TopLeft;
      else if (cadena.compare("top-right") == 0)
        m_bannerPosition = BP_TopRight;
      else if (cadena.compare("bottom-left") == 0)
        m_bannerPosition = BP_BottomLeft;
      else if (cadena.compare("bottom-right") == 0)
        m_bannerPosition = BP_BottomRight;
      else
        m_bannerPosition = BP_TopLeft;
    }

    if (xml.GetSectionAttribute("configuration/progressbanner", "color", cadena))
    {
      if (cadena.length() == 7 && cadena[0] == '#')
      {
        string colorHexadecimal = cadena.substr(1,6).c_str();
        char* invalid = NULL; 
        unsigned long colorDecimal = strtoul(colorHexadecimal.c_str(), &invalid, 16);
        if (invalid == &colorHexadecimal.c_str()[6])
        {
          m_bannerTextColor = D3DCOLOR_RGBA((colorDecimal >> 16) & 0xFF, (colorDecimal >> 8) & 0xFF, colorDecimal & 0xFF, 0xFF);
        }
      }
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // Read statistics plugins options

    string pluginFileName;
    string pluginCounters;
    
    char sectionName[256];
    unsigned int pluginCounter = 0;
    sprintf(sectionName, "configuration/statisticsplugins/plugin%u", pluginCounter);
    while (xml.GetSectionAttribute(sectionName, "filename", pluginFileName))
    {
      if (xml.GetSectionAttribute(sectionName, "counters", pluginCounters))
      {
        StatisticsPlugin plugin;
        
        plugin.PluginFileName = pluginFileName;
        
        static const rpattern patro_divisio(",");
        split_results results;
        if (patro_divisio.split(pluginCounters, results) > 0)
        {
          for (split_results::iterator it=results.begin(); it != results.end(); it++)
          {
            plugin.Counters.push_back(atoi((*it).c_str()));
          }
        }
        
        if (plugin.Counters.size() > 0)
        {
          AddPlugin(plugin);
        }
      }
      sprintf(sectionName, "configuration/statisticsplugins/plugin%u", ++pluginCounter);
    }
    
    //////////////////////////////////////////////////////////////////////////////
    
    return true;
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXInterceptorOptions::SaveXML(const std::string& filename)
{
  XMLConfig xml(filename, true);
  xml.FillDefault();

  xml.AddSectionText("configuration/destinationdir", m_destinationPath);

  xml.AddSection("configuration/storage");
  xml.AddSectionAttribute("configuration/storage", "compression", (m_compression ? "true" : "false"));

  xml.AddSection("configuration/progressbanner");
  xml.AddSectionAttribute("configuration/progressbanner", "show", m_bannerShow ? "true" : "false");
  switch (m_bannerPosition)
  {
  case BP_BottomRight:
    xml.AddSectionAttribute("configuration/progressbanner", "position", "bottom-right");
    break;
  case BP_BottomLeft:
    xml.AddSectionAttribute("configuration/progressbanner", "position", "bottom-left");
    break;
  case BP_TopRight:
    xml.AddSectionAttribute("configuration/progressbanner", "position", "top-right");
    break;
  case BP_TopLeft:
  default:
    xml.AddSectionAttribute("configuration/progressbanner", "position", "top-left");
    break;
  }
  ostringstream bannerTextColor;
  bannerTextColor << "#";
  bannerTextColor << hex << uppercase << setw(2) << setfill('0') << (DWORD) ((m_bannerTextColor >> 16) & 0xFF);
  bannerTextColor << hex << uppercase << setw(2) << setfill('0') << (DWORD) ((m_bannerTextColor >>  8) & 0xFF);
  bannerTextColor << hex << uppercase << setw(2) << setfill('0') << (DWORD) ((m_bannerTextColor >>  0) & 0xFF);
  xml.AddSectionAttribute("configuration/progressbanner", "color", bannerTextColor.str());

  //////////////////////////////////////////////////////////////////////////////
  // Save statistics plugins options

  xml.AddSection("configuration/statisticsplugins");
  for (unsigned int i=0; i < (unsigned int) m_plugins.size(); ++i)
  {
    if (!m_plugins[i].Counters.size())
    {
      continue;
    }
    
    char sectionName[256];
    sprintf(sectionName, "configuration/statisticsplugins/plugin%u", i);
    
    xml.AddSection(sectionName);
    xml.AddSectionAttribute(sectionName, "filename", m_plugins[i].PluginFileName);

    string counters;
    for (unsigned int j=0; j < (unsigned int) m_plugins[i].Counters.size(); ++j)
    {
      if (j > 0) counters += ", ";
      char counterID[16];
      sprintf(counterID, "%u", m_plugins[i].Counters[j]);
      counters += counterID;
    }

    xml.AddSectionAttribute(sectionName, "counters", counters);
  }

  //////////////////////////////////////////////////////////////////////////////

  return xml.Save();
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXInterceptorOptions::GetPluginCount()
{
  return (unsigned int) m_plugins.size();
}

////////////////////////////////////////////////////////////////////////////////

bool DXInterceptorOptions::GetPlugin(unsigned int number, StatisticsPlugin& plugin)
{
  if (number < (unsigned int) m_plugins.size())
  {
    plugin = m_plugins[number];
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorOptions::AddPlugin(const StatisticsPlugin& plugin)
{
  m_plugins.push_back(plugin);
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorOptions::ClearPlugins()
{
  m_plugins.clear();
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorOptions::CloneObjects(const DXInterceptorOptions& orig, DXInterceptorOptions& dest)
{
  dest.m_destinationPath = orig.m_destinationPath;
  dest.m_compression = orig.m_compression;
  dest.m_bannerShow = orig.m_bannerShow;
  dest.m_bannerPosition = orig.m_bannerPosition;
  dest.m_bannerTextColor = orig.m_bannerTextColor;

  dest.m_plugins.clear();
  copy(orig.m_plugins.begin(), orig.m_plugins.end(), back_inserter(dest.m_plugins));
}

////////////////////////////////////////////////////////////////////////////////
