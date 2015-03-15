////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items/TypeSpecificCode.h"
#include "Items/StructSpecificCode.h"
#include "Items/ClassSpecificCode.h"
#include "Config/GeneratorConfiguration.h"

using namespace std;
using namespace dxcodegen::Items;
using namespace dxcodegen::Config;

////////////////////////////////////////////////////////////////////////////////

GeneratorConfiguration::GeneratorConfiguration()
{
}

////////////////////////////////////////////////////////////////////////////////

GeneratorConfiguration::~GeneratorConfiguration()
{
}

////////////////////////////////////////////////////////////////////////////////

void GeneratorConfiguration::Clear()
{
  m_outputPath.clear();
  m_wrapperSuffix.clear();
  m_stubSuffix.clear();
  m_wrapperBaseClass.clear();
  m_stubBaseClass.clear();
  m_mapClassesSpecificCode.clear();
}

////////////////////////////////////////////////////////////////////////////////

void GeneratorConfiguration::SetOutputPath(const string& path)
{
  m_outputPath = path;
}

////////////////////////////////////////////////////////////////////////////////

string& GeneratorConfiguration::GetOutputPath()
{
  return m_outputPath;
}

////////////////////////////////////////////////////////////////////////////////

void GeneratorConfiguration::SetWrapperSuffix(const string& wrapperSuffix)
{
  m_wrapperSuffix = wrapperSuffix;
}

////////////////////////////////////////////////////////////////////////////////

string& GeneratorConfiguration::GetWrapperSuffix()
{
  return m_wrapperSuffix;
}

////////////////////////////////////////////////////////////////////////////////

void GeneratorConfiguration::SetStubSuffix(const string& stubSuffix)
{
  m_stubSuffix = stubSuffix;
}

////////////////////////////////////////////////////////////////////////////////

string& GeneratorConfiguration::GetStubSuffix()
{
  return m_stubSuffix;
}

////////////////////////////////////////////////////////////////////////////////

void GeneratorConfiguration::SetWrapperBaseClass(const string& wrapperBaseClass)
{
  m_wrapperBaseClass = wrapperBaseClass;
}

////////////////////////////////////////////////////////////////////////////////

string& GeneratorConfiguration::GetWrapperBaseClass()
{
  return m_wrapperBaseClass;
}

////////////////////////////////////////////////////////////////////////////////

void GeneratorConfiguration::SetStubBaseClass(const string& stubBaseClass)
{
  m_stubBaseClass = stubBaseClass;
}

////////////////////////////////////////////////////////////////////////////////

string& GeneratorConfiguration::GetStubBaseClass()
{
  return m_stubBaseClass;
}

////////////////////////////////////////////////////////////////////////////////

void GeneratorConfiguration::AddTypeSpecificCode(TypeSpecificCodePtr tipo)
{
  m_mapTypesSpecificCode[tipo->GetName()] = tipo;
}

////////////////////////////////////////////////////////////////////////////////

TypeSpecificCodePtr GeneratorConfiguration::GetTypeSpecificCode(const std::string& name)
{
  map<string, TypeSpecificCodePtr>::iterator tipo = m_mapTypesSpecificCode.find(name);
  if (tipo != m_mapTypesSpecificCode.end())
  {
    return (*tipo).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

void GeneratorConfiguration::AddStructSpecificCode(StructSpecificCodePtr estructura)
{
  m_mapStructsSpecificCode[estructura->GetName()] = estructura;
}

////////////////////////////////////////////////////////////////////////////////

StructSpecificCodePtr GeneratorConfiguration::GetStructSpecificCode(const std::string& name)
{
  map<string, StructSpecificCodePtr>::iterator estructura = m_mapStructsSpecificCode.find(name);
  if (estructura != m_mapStructsSpecificCode.end())
  {
    return (*estructura).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

void GeneratorConfiguration::AddClassSpecificCode(ClassSpecificCodePtr classe)
{
  m_mapClassesSpecificCode[classe->GetName()] = classe;
}

////////////////////////////////////////////////////////////////////////////////

ClassSpecificCodePtr GeneratorConfiguration::GetClassSpecificCode(const std::string& name)
{
  map<string, ClassSpecificCodePtr>::iterator classe = m_mapClassesSpecificCode.find(name);
  if (classe != m_mapClassesSpecificCode.end())
  {
    return (*classe).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
