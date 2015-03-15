////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <shlwapi.h>
#include "tinyxml.h"
#include "XMLConfig.h"
#include "DXPlayerOptions.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

DXPlayerOptions::DXPlayerOptions()
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerOptions::DXPlayerOptions(const DXPlayerOptions& options)
{
  CloneObjects(options, *this);
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerOptions::~DXPlayerOptions()
{
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerOptions& DXPlayerOptions::operator = (const DXPlayerOptions& options)
{
  CloneObjects(options, *this);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerOptions::Clear()
{
  m_destinationPath = "";
  m_screenshotFormat = SSF_BMP;
}

////////////////////////////////////////////////////////////////////////////////

string DXPlayerOptions::GetDestinationPath() const
{
  return m_destinationPath;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerOptions::SetDestinationPath(const string& value)
{
  m_destinationPath = value;
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerOptions::ScreenshotFormat DXPlayerOptions::GetScreenshotFormat() const
{
  return m_screenshotFormat;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerOptions::SetScreenshotFormat(ScreenshotFormat value)
{
  m_screenshotFormat = value;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerOptions::LoadXML(const std::string& filename)
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

    if (xml.GetSectionText("configuration/screenshotformat", cadena))
    {
      if (cadena.compare("bmp") == 0)
        m_screenshotFormat = SSF_BMP;
      else if (cadena.compare("jpg") == 0)
        m_screenshotFormat = SSF_JPG;
      else if (cadena.compare("tga") == 0)
        m_screenshotFormat = SSF_TGA;
      else if (cadena.compare("png") == 0)
        m_screenshotFormat = SSF_PNG;
      else if (cadena.compare("dds") == 0)
        m_screenshotFormat = SSF_DDS;
      else if (cadena.compare("ppm") == 0)
        m_screenshotFormat = SSF_PPM;
      else if (cadena.compare("dib") == 0)
        m_screenshotFormat = SSF_DIB;
      else if (cadena.compare("hdr") == 0)
        m_screenshotFormat = SSF_HDR;
      else if (cadena.compare("pfm") == 0)
        m_screenshotFormat = SSF_PFM;
      else
        m_screenshotFormat = SSF_BMP;
    }
    
    return true;
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerOptions::SaveXML(const std::string& filename)
{
  XMLConfig xml(filename, true);
  xml.FillDefault();

  xml.AddSectionText("configuration/destinationdir", m_destinationPath);

  switch (m_screenshotFormat)
  {
  default:
  case SSF_BMP:
    xml.AddSectionText("configuration/screenshotformat", "bmp");
    break;
  case SSF_JPG:
    xml.AddSectionText("configuration/screenshotformat", "jpg");
    break;
  case SSF_TGA:
    xml.AddSectionText("configuration/screenshotformat", "tga");
    break;
  case SSF_PNG:
    xml.AddSectionText("configuration/screenshotformat", "png");
    break;
  case SSF_DDS:
    xml.AddSectionText("configuration/screenshotformat", "dds");
    break;
  case SSF_PPM:
    xml.AddSectionText("configuration/screenshotformat", "ppm");
    break;
  case SSF_DIB:
    xml.AddSectionText("configuration/screenshotformat", "dib");
    break;
  case SSF_HDR:
    xml.AddSectionText("configuration/screenshotformat", "hdr");
    break;
  case SSF_PFM:
    xml.AddSectionText("configuration/screenshotformat", "pfm");
    break;
  }

  return xml.Save();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerOptions::CloneObjects(const DXPlayerOptions& orig, DXPlayerOptions& dest)
{
  dest.m_destinationPath = orig.m_destinationPath;
  dest.m_screenshotFormat = orig.m_screenshotFormat;
}

////////////////////////////////////////////////////////////////////////////////
