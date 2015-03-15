////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items/CppMacro.h"
#include "Config/ParserConfiguration.h"

using namespace std;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;

////////////////////////////////////////////////////////////////////////////////

ParserConfiguration::ParserConfiguration()
{
}

////////////////////////////////////////////////////////////////////////////////

ParserConfiguration::~ParserConfiguration()
{
}

////////////////////////////////////////////////////////////////////////////////

void ParserConfiguration::Clear()
{
  m_lstFiles.clear();
  m_lstEnumsInclude.clear();
  m_lstEnumsExclude.clear();
  m_lstStructsInclude.clear();
  m_lstStructsExclude.clear();
  m_lstClassesInclude.clear();
  m_lstClassesExclude.clear();
  m_lstMacros.clear();

  m_mapEnumsInclude.clear();
  m_mapEnumsExclude.clear();
  m_mapStructsInclude.clear();
  m_mapStructsExclude.clear();
  m_mapClassesInclude.clear();
  m_mapClassesExclude.clear();
}

////////////////////////////////////////////////////////////////////////////////

void ParserConfiguration::AddFile(const string& filename)
{
  m_lstFiles.push_back(filename);
}

////////////////////////////////////////////////////////////////////////////////

vector<string>& ParserConfiguration::GetFiles()
{
  return m_lstFiles;
}

////////////////////////////////////////////////////////////////////////////////

void ParserConfiguration::AddEnum(const string& name, bool exclude)
{
  pair<string,string> ins_elem(name, name);
  pair<map<string,string>::iterator, bool> ins_res;

  if (exclude)
  {
    ins_res = m_mapEnumsExclude.insert(ins_elem);
    if (!ins_res.second)
    {
      m_lstEnumsExclude.push_back(name);
    }
  }
  else
  {
    ins_res = m_mapEnumsInclude.insert(ins_elem);
    if (!ins_res.second)
    {
      m_lstEnumsInclude.push_back(name);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

vector<string>& ParserConfiguration::GetEnumsToInclude()
{
  return m_lstEnumsInclude;
}

////////////////////////////////////////////////////////////////////////////////

vector<string>& ParserConfiguration::GetEnumsToExclude()
{
  return m_lstEnumsExclude;
}

////////////////////////////////////////////////////////////////////////////////

void ParserConfiguration::AddStruct(const string& name, bool exclude)
{
  pair<string,string> ins_elem(name, name);
  pair<map<string,string>::iterator, bool> ins_res;
  
  if (exclude)
  {
    ins_res = m_mapStructsExclude.insert(ins_elem);
    if (!ins_res.second)
    {
      m_lstStructsExclude.push_back(name);
    }
  }
  else
  {
    ins_res = m_mapStructsInclude.insert(ins_elem);
    if (!ins_res.second)
    {
      m_lstStructsInclude.push_back(name);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

vector<string>& ParserConfiguration::GetStructsToInclude()
{
  return m_lstStructsInclude;
}

////////////////////////////////////////////////////////////////////////////////

vector<string>& ParserConfiguration::GetStructsToExclude()
{
  return m_lstStructsExclude;
}

////////////////////////////////////////////////////////////////////////////////

void ParserConfiguration::AddClass(const string& name, bool exclude)
{
  pair<string,string> ins_elem(name, name);
  pair<map<string,string>::iterator, bool> ins_res;
  
  if (exclude)
  {
    ins_res = m_mapClassesExclude.insert(ins_elem);
    if (!ins_res.second)
    {
      m_lstClassesExclude.push_back(name);
    }
  }
  else
  {
    ins_res = m_mapClassesInclude.insert(ins_elem);
    if (!ins_res.second)
    {
      m_lstClassesInclude.push_back(name);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

vector<string>& ParserConfiguration::GetClassesToInclude()
{
  return m_lstClassesInclude;
}

////////////////////////////////////////////////////////////////////////////////

vector<string>& ParserConfiguration::GetClassesToExclude()
{
  return m_lstClassesExclude;
}

////////////////////////////////////////////////////////////////////////////////

void ParserConfiguration::AddMacro(const CppMacroPtr macro)
{
  m_lstMacros.push_back(macro);
}

////////////////////////////////////////////////////////////////////////////////

vector<CppMacroPtr>& ParserConfiguration::GetMacros()
{
  return m_lstMacros;
}

////////////////////////////////////////////////////////////////////////////////

bool ParserConfiguration::IsEnumParseCandidate(const string& name)
{	
  if (m_mapEnumsInclude.empty() && m_mapEnumsExclude.empty())
  {
    return true;
  }
  else
  {
    map<string,string>::const_iterator found;
    found = m_mapEnumsExclude.find(name);
    if (found != m_mapEnumsExclude.end())
    {
      return false;
    }
    else
    {
      if (!m_mapEnumsInclude.empty())
      {
        found = m_mapEnumsInclude.find(name);
        return (found != m_mapEnumsInclude.end());
      }
      else
      {
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool ParserConfiguration::IsStructParseCandidate(const string& name)
{	
  if (m_mapStructsInclude.empty() && m_mapStructsExclude.empty())
  {
    return true;
  }
  else
  {
    map<string,string>::const_iterator found;
    found = m_mapStructsExclude.find(name);
    if (found != m_mapStructsExclude.end())
    {
      return false;
    }
    else
    {
      if (!m_mapStructsInclude.empty())
      {
        found = m_mapStructsInclude.find(name);
        return (found != m_mapStructsInclude.end());
      }
      else
      {
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool ParserConfiguration::IsClassParseCandidate(const string& name)
{	
  if (m_mapClassesInclude.empty() && m_mapClassesExclude.empty())
  {
    return true;
  }
  else
  {
    map<string,string>::const_iterator found;
    found = m_mapClassesExclude.find(name);
    if (found != m_mapClassesExclude.end())
    {
      return false;
    }
    else
    {
      if (!m_mapClassesInclude.empty())
      {
        found = m_mapClassesInclude.find(name);
        return (found != m_mapClassesInclude.end());
      }
      else
      {
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
