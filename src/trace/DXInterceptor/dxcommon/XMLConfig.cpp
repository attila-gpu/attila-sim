////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <regexpr2.h>
#include <tinyxml.h>
#include "XMLConfig.h"

using namespace std;
using namespace regex;

////////////////////////////////////////////////////////////////////////////////

XMLConfig::XMLConfig(const string& filename, bool createIfNotExists) :
m_filename(filename)
{
  bool loadOkay = m_xmlDocument.LoadFile(m_filename);
  if (!loadOkay)
  {
    // ErrorId = 2: File not found
    if (m_xmlDocument.ErrorId()==2 && createIfNotExists)
    {
      FillDefault();
      Save();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool XMLConfig::AddHeader()
{
  if (!ExistsHeader())
  {
    m_xmlDocument.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", "yes"));
  }
  if (!m_xmlDocument.RootElement())
  {
    TiXmlElement* section = new TiXmlElement("configuration");
    m_xmlDocument.LinkEndChild(section);
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool XMLConfig::AddSection(const string& sectionName)
{
  static const rpattern patro_divisio("/");
  split_results results;
  if (patro_divisio.split(sectionName, results) > 0)
  {
    TiXmlNode* node = NULL;
    split_results::iterator it;

    for (it=results.begin(); it != results.end(); it++)
    {
      if (it == results.begin())
      {
        node = m_xmlDocument.RootElement();
      }
      else
      {
        if (node->FirstChild(*it))
        {
          node = node->FirstChild(*it);
        }        
      }

      if (node->ToElement()->ValueStr() != *it)
      {
        node = node->LinkEndChild(new TiXmlElement(*it));
      }
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool XMLConfig::AddSectionText(const string& sectionName, const string& sectionText)
{
  TiXmlNode* root = GetSection(sectionName);
  if (!root)
  {
    AddSection(sectionName);
    root = GetSection(sectionName);
  }
  
  if (root)
  {
    if (!root->FirstChild())
    {
      root->LinkEndChild(new TiXmlText(""));
    }
    root->FirstChild()->ToText()->SetValue(sectionText);
    
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool XMLConfig::AddSectionAttribute(const string& sectionName, const string& attribName, const string& attribValue)
{
  TiXmlNode* root = GetSection(sectionName);
  if (root)
  {
    TiXmlElement* section = root->ToElement();
    section->SetAttribute(attribName, attribValue);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool XMLConfig::GetSectionText(const string& sectionName, string& sectionText)
{
  sectionText.clear();
  TiXmlNode* node = GetSection(sectionName);
  if (node)
  {
    if (node->ToElement()->GetText())
    {
      sectionText = node->ToElement()->GetText();
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool XMLConfig::GetSectionAttribute(const string& sectionName, const string& attribName, string& attribValue)
{
  attribValue.clear();  
  TiXmlNode* node = GetSection(sectionName);
  if (node)
  {
    if (node->ToElement()->Attribute(attribName))
    {
      attribValue = *node->ToElement()->Attribute(attribName);
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool XMLConfig::ExistsHeader()
{
  if (m_xmlDocument.NoChildren())
  {
    return false;
  }
  else
  {
    TiXmlNode* node = m_xmlDocument.FirstChild();
    if (node->Type() != TiXmlNode::DECLARATION)
    {
      return false;
    }
    else
    {
      TiXmlDeclaration* declaration = node->ToDeclaration();

      string cadena = declaration->Version();
      if (cadena != "1.0")
      {
        node->Parent()->RemoveChild(node);
        return false;
      }

      cadena = declaration->Encoding();
      if (cadena != "utf-8")
      {
        node->Parent()->RemoveChild(node);
        return false;
      }

      cadena = declaration->Standalone();
      if (cadena != "yes")
      {
        node->Parent()->RemoveChild(node);
        return false;
      }

      return true;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

TiXmlNode* XMLConfig::GetSection(const string& sectionName)
{
  TiXmlNode* node = NULL;
  static const rpattern patro_divisio("/");
  split_results results;
  if (patro_divisio.split(sectionName, results) > 0)
  {
    bool problem = m_xmlDocument.NoChildren();
    split_results::iterator it;

    for (it=results.begin(); it != results.end() && !problem; it++)
    {
      if (it == results.begin())
        node = m_xmlDocument.RootElement();
      else
        node = node->FirstChild(*it);

      if (node)
        problem = (node->ToElement()->ValueStr() != *it);
      else
        problem = true;
    }

    if (problem)
      node = NULL;
  }

  return node;
}

////////////////////////////////////////////////////////////////////////////////

void XMLConfig::Clear()
{
  m_xmlDocument.Clear();
}

////////////////////////////////////////////////////////////////////////////////

bool XMLConfig::FillDefault()
{
  Clear();
  AddHeader();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool XMLConfig::Save()
{
  if (!m_filename.empty())
  {
    AddHeader();
    m_xmlDocument.SaveFile(m_filename);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
