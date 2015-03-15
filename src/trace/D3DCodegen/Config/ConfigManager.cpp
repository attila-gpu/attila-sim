/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXCodeGenException.hpp"
#include "Utilities/FileSystem.hpp"
#include "Utilities/String.hpp"
#include "Utilities/System.hpp"
#include "Items/CppMacro.hpp"
#include "Config/ParserConfiguration.hpp"
#include "Config/GeneratorConfiguration.hpp"
#include "Config/ConfigManager.hpp"

using namespace std;
using namespace regex;
using namespace dxcodegen;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;

////////////////////////////////////////////////////////////////////////////////

ConfigManager::ConfigManager(const string& filename, bool verbose, bool createIfNotExists) :
m_filename(filename),
m_verbose(verbose)
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
    else
    {
      DXCodeGenException e("could'nt load '" + m_filename + "'. Error='" + m_xmlDocument.ErrorDesc() + "'");
      throw e; 
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::GetParserConfiguration(ParserConfiguration& config)
{
  config.Clear();
  
  if (m_verbose)
  {
    cout << "Loading configuration from '" << m_filename << "'" << endl;
    cout << "----------------------------";
    for (size_t i=0; i < m_filename.length(); i++)
    {
      cout << "-";
    }
    cout << "-" << endl;
  }
  
  AddParserFiles(config);
  AddParserEnums(config);
  AddParserStructs(config);
  AddParserClasses(config);
  AddParserMacros(config);

  if (m_verbose)
  {
    cout << "----------------------------";
    for (size_t i=0; i < m_filename.length(); i++)
    {
      cout << "-";
    }
    cout << "-" << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::GetGeneratorConfiguration(GeneratorConfiguration& config)
{
  TiXmlNode* root = GetSection("configuration/generator");
  if (root)
  {
    string outputpath;
    TiXmlElement* generator = root->ToElement();
    if (generator->Attribute("outputpath"))
    {
      config.SetOutputPath(generator->Attribute("outputpath"));
    }
  }
}
////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddParserFiles(ParserConfiguration& config)
{
  TiXmlNode* root = GetSection("configuration/parser");
  if (root)
  {
    TiXmlNode* node = NULL;
    while (node = root->IterateChildren("files", node))
    {
      string basepath;
      TiXmlElement* files = node->ToElement();
      if (files->Attribute("basepath"))
      {
        basepath = files->Attribute("basepath");
      }

      TiXmlNode* child = NULL;
      while (child = node->IterateChildren("file", child))
      {
        TiXmlElement* file = child->ToElement();
        if (file->GetText())
        {
          string filename = file->GetText();
          Utilities::String::TrimString(filename);
          if (!filename.empty())
          {
            if (m_verbose)
            {
              cout << "Added file '" << basepath + filename << "' to parse" << endl;
            }
            config.AddFile(basepath + filename);
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddParserEnums(ParserConfiguration& config)
{
  vector<string>* vec_str;
  vector<string>::iterator it;
  
  vec_str = GetTextList("configuration/parser/enums", "include");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added enum '" << *it << "' to include list" << endl;
    }
    config.AddEnum(*it, false);
  }
  delete vec_str;

  vec_str = GetTextList("configuration/parser/enums", "exclude");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added enum '" << *it << "' to exclude list" << endl;
    }
    config.AddEnum(*it, true);
  }
  delete vec_str;
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddParserStructs(ParserConfiguration& config)
{
  vector<string>* vec_str;
  vector<string>::iterator it;

  vec_str = GetTextList("configuration/parser/structs", "include");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added struct '" << *it << "' to include list" << endl;
    }
    config.AddStruct(*it, false);
  }
  delete vec_str;

  vec_str = GetTextList("configuration/parser/structs", "exclude");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added struct '" << *it << "' to exclude list" << endl;
    }
    config.AddStruct(*it, true);
  }
  delete vec_str;
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddParserClasses(ParserConfiguration& config)
{
  vector<string>* vec_str;
  vector<string>::iterator it;

  vec_str = GetTextList("configuration/parser/classes", "include");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added class '" << *it << "' to include list" << endl;
    }
    config.AddClass(*it, false);
  }
  delete vec_str;

  vec_str = GetTextList("configuration/parser/classes", "exclude");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added class '" << *it << "' to exclude list" << endl;
    }
    config.AddClass(*it, true);
  }
  delete vec_str;
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddParserMacros(ParserConfiguration& config)
{
  TiXmlNode* root = GetSection("configuration/parser/macros");
  if (root)
  {
    TiXmlNode* node = NULL;
    while (node = root->IterateChildren("macro", node))
    {
      const char* p;
      string left;
      p = node->FirstChild("left")->ToElement()->GetText();
      if (p) left = p;
      string right;
      p = node->FirstChild("right")->ToElement()->GetText();
      if (p) right = p;
      if (!left.empty())
      {
        CppMacro macro(left, right);
        if (m_verbose)
        {
          cout << "Added macro '" << macro.Left() << "'" << endl;
        }
        config.AddMacro(macro);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::AddHeader()
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

bool ConfigManager::AddSection(string sectionParent, string sectionName)
{
  TiXmlNode* root = GetSection(sectionParent);
  if (root)
  {
    TiXmlElement* childSection = new TiXmlElement(sectionName);
    root->LinkEndChild(childSection);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::AddSection(string sectionParent, string sectionName, string sectionText)
{
  TiXmlNode* root = GetSection(sectionParent);
  if (root)
  {
    TiXmlElement* childSection = new TiXmlElement(sectionName);
    childSection->LinkEndChild(new TiXmlText(sectionText));
    root->LinkEndChild(childSection);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::AddSectionAttribute(string sectionName, string attribName, string attribValue)
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

bool ConfigManager::ExistsHeader()
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

TiXmlNode* ConfigManager::GetSection(string sectionName)
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

vector<string>* ConfigManager::GetTextList(string sectionName, string subSectionName)
{
  vector<string>* llista = new vector<string>();

  TiXmlNode* root = GetSection(sectionName);
  if (root)
  {
    TiXmlNode* node = NULL;
    while (node = root->IterateChildren(subSectionName, node))
    {
      TiXmlElement* elem = node->ToElement();
      if (elem->GetText())
      {
        string cadena = elem->GetText();
        Utilities::String::TrimString(cadena);
        if (!cadena.empty())
        {
          llista->push_back(cadena);
        }
      }
    }
  }

  return llista;
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::Clear()
{
  m_xmlDocument.Clear();
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::FillDefault()
{
  Clear();

  AddHeader();

  AddSection("configuration", "parser");
  AddSection("configuration/parser", "files");
  AddSectionAttribute("configuration/parser/files", "basepath", GetDirectXSDKPath());

  AddSection("configuration/parser/files", "file", "d3d9.h");
  AddSection("configuration/parser/files", "file", "d3d9caps.h");
  AddSection("configuration/parser/files", "file", "d3d9types.h");

  AddSection("configuration/parser", "enums");
  AddSection("configuration/parser/enums", "include");
  AddSection("configuration/parser/enums", "exclude");

  AddSection("configuration/parser", "structs");
  AddSection("configuration/parser/structs", "include");
  AddSection("configuration/parser/structs", "exclude");

  AddSection("configuration/parser", "classes");
  AddSection("configuration/parser/classes", "include");
  AddSection("configuration/parser/classes", "exclude");

  AddSection("configuration/parser", "macros");
  AddSection("configuration/parser/macros", "macro");
  AddSection("configuration/parser/macros/macro", "left", "");
  AddSection("configuration/parser/macros/macro", "right", "");

  AddSection("configuration", "generator");
  AddSectionAttribute("configuration/generator", "outputpath", "");

  return false;
}

////////////////////////////////////////////////////////////////////////////////

string ConfigManager::GetDirectXSDKPath()
{
  string pathSDK;
  Utilities::System::ReadEnvironmentVariable("DXSDK_DIR", pathSDK);
  if (!pathSDK.empty() && Utilities::FileSystem::DirectoryExists(pathSDK))
  {
    if (pathSDK[pathSDK.length()-1] != '\\')
    {
      pathSDK += "\\";
    }
    pathSDK += "include\\";
    if (!Utilities::FileSystem::DirectoryExists(pathSDK))
    {
      pathSDK.clear();
    }
  }
  else
  {
    pathSDK.clear();
  }
  return pathSDK;
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::Save()
{
  if (!m_filename.empty())
  {
    m_xmlDocument.SaveFile(m_filename);
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
