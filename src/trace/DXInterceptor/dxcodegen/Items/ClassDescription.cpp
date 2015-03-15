////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items/SmartPointer.h"
#include "Items/MethodDescriptionParam.h"
#include "Items/MethodDescription.h"
#include "Items/ClassDescription.h"

using namespace std;
using namespace dxcodegen::Items;

////////////////////////////////////////////////////////////////////////////////

ClassDescription::ClassDescription()
{
}

////////////////////////////////////////////////////////////////////////////////

ClassDescription::~ClassDescription()
{
}

////////////////////////////////////////////////////////////////////////////////

void ClassDescription::SetName(const string& name)
{
  m_name = name;
}

////////////////////////////////////////////////////////////////////////////////

string& ClassDescription::GetName()
{
  return m_name;
}

////////////////////////////////////////////////////////////////////////////////

void ClassDescription::AddMethod(const MethodDescriptionPtr method)
{
  pair<map<string, MethodDescriptionPtr>::iterator, bool> ins_res;
  ins_res = m_mapMethods.insert(pair<string, MethodDescriptionPtr>(method->GetName(), method));
  if (ins_res.second)
  {
    m_lstMethods.push_back(method);
  }
}

////////////////////////////////////////////////////////////////////////////////

unsigned int ClassDescription::GetMethodsCount()
{
  return (unsigned int) m_lstMethods.size();
}

////////////////////////////////////////////////////////////////////////////////

MethodDescriptionPtr ClassDescription::GetMethod(unsigned int position)
{
  return m_lstMethods[position];
}

////////////////////////////////////////////////////////////////////////////////

MethodDescriptionPtr ClassDescription::GetMethod(const string& name)
{
  map<string, MethodDescriptionPtr>::iterator method = m_mapMethods.find(name);
  if (method != m_mapMethods.end())
  {
    return (*method).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
