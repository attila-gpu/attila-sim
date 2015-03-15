////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items/MethodSpecificCode.h"
#include "Items/ClassSpecificCode.h"

using namespace std;
using namespace dxcodegen::Items;

////////////////////////////////////////////////////////////////////////////////

ClassSpecificCode::ClassSpecificCode(const std::string& name) :
m_name(name)
{
}

////////////////////////////////////////////////////////////////////////////////

ClassSpecificCode::~ClassSpecificCode()
{
}

////////////////////////////////////////////////////////////////////////////////

std::string& ClassSpecificCode::GetName()
{
  return m_name;
}

////////////////////////////////////////////////////////////////////////////////

void ClassSpecificCode::AddWrapperAttribute(AttributeSpecificCodePtr attrib)
{
  m_mapWrapperAttributesSpecificCode[attrib->GetName()] = attrib;
}

////////////////////////////////////////////////////////////////////////////////

AttributeSpecificCodePtr ClassSpecificCode::GetWrapperAttribute(const string& name)
{
  map<string, AttributeSpecificCodePtr>::iterator type = m_mapWrapperAttributesSpecificCode.find(name);
  if (type != m_mapWrapperAttributesSpecificCode.end())
  {
    return (*type).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

vector<string>* ClassSpecificCode::GetWrapperAttributeNames()
{
  vector<string>* v_names = new vector<string>();

  map<string, AttributeSpecificCodePtr>::iterator it;
  for (it = m_mapWrapperAttributesSpecificCode.begin(); it != m_mapWrapperAttributesSpecificCode.end(); it++)
  {
    v_names->push_back((*it).first);
  }

  return v_names;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int ClassSpecificCode::GetWrapperAttributeCount()
{
  return (unsigned int) m_mapWrapperAttributesSpecificCode.size();
}

////////////////////////////////////////////////////////////////////////////////

void ClassSpecificCode::AddStubAttribute(AttributeSpecificCodePtr attrib)
{
  m_mapStubAttributesSpecificCode[attrib->GetName()] = attrib;
}

////////////////////////////////////////////////////////////////////////////////

AttributeSpecificCodePtr ClassSpecificCode::GetStubAttribute(const string& name)
{
  map<string, AttributeSpecificCodePtr>::iterator type = m_mapStubAttributesSpecificCode.find(name);
  if (type != m_mapStubAttributesSpecificCode.end())
  {
    return (*type).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

vector<string>* ClassSpecificCode::GetStubAttributeNames()
{
  vector<string>* v_names = new vector<string>();

  map<string, AttributeSpecificCodePtr>::iterator it;
  for (it = m_mapStubAttributesSpecificCode.begin(); it != m_mapStubAttributesSpecificCode.end(); it++)
  {
    v_names->push_back((*it).first);
  }

  return v_names;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int ClassSpecificCode::GetStubAttributeCount()
{
  return (unsigned int) m_mapStubAttributesSpecificCode.size();
}

////////////////////////////////////////////////////////////////////////////////

void ClassSpecificCode::AddWrapperNewMethod(NewMethodSpecificCodePtr method)
{
  m_mapWrapperNewMethodsSpecificCode[method->GetName()] = method;
}

////////////////////////////////////////////////////////////////////////////////

NewMethodSpecificCodePtr ClassSpecificCode::GetWrapperNewMethod(const std::string& name)
{
  map<string, NewMethodSpecificCodePtr>::iterator method = m_mapWrapperNewMethodsSpecificCode.find(name);
  if (method != m_mapWrapperNewMethodsSpecificCode.end())
  {
    return (*method).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

vector<string>* ClassSpecificCode::GetWrapperNewMethodNames()
{
  vector<string>* v_names = new vector<string>();

  map<string, NewMethodSpecificCodePtr>::iterator it;
  for (it = m_mapWrapperNewMethodsSpecificCode.begin(); it != m_mapWrapperNewMethodsSpecificCode.end(); it++)
  {
    v_names->push_back((*it).first);
  }

  return v_names;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int ClassSpecificCode::GetWrapperNewMethodCount()
{
  return (unsigned int) m_mapWrapperNewMethodsSpecificCode.size();
}

////////////////////////////////////////////////////////////////////////////////

void ClassSpecificCode::AddStubNewMethod(NewMethodSpecificCodePtr method)
{
  m_mapStubNewMethodsSpecificCode[method->GetName()] = method;
}

////////////////////////////////////////////////////////////////////////////////

NewMethodSpecificCodePtr ClassSpecificCode::GetStubNewMethod(const std::string& name)
{
  map<string, NewMethodSpecificCodePtr>::iterator method = m_mapStubNewMethodsSpecificCode.find(name);
  if (method != m_mapStubNewMethodsSpecificCode.end())
  {
    return (*method).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

vector<string>* ClassSpecificCode::GetStubNewMethodNames()
{
  vector<string>* v_names = new vector<string>();

  map<string, NewMethodSpecificCodePtr>::iterator it;
  for (it = m_mapStubNewMethodsSpecificCode.begin(); it != m_mapStubNewMethodsSpecificCode.end(); it++)
  {
    v_names->push_back((*it).first);
  }

  return v_names;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int ClassSpecificCode::GetStubNewMethodCount()
{
  return (unsigned int) m_mapStubNewMethodsSpecificCode.size();
}

////////////////////////////////////////////////////////////////////////////////

void ClassSpecificCode::AddMethod(MethodSpecificCodePtr method)
{
  m_mapMethodsSpecificCode[method->GetName()] = method;
}

////////////////////////////////////////////////////////////////////////////////

MethodSpecificCodePtr ClassSpecificCode::GetMethod(const std::string& name)
{
  map<string, MethodSpecificCodePtr>::iterator method = m_mapMethodsSpecificCode.find(name);
  if (method != m_mapMethodsSpecificCode.end())
  {
    return (*method).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

vector<string>* ClassSpecificCode::GetMethodNames()
{
  vector<string>* v_names = new vector<string>();

  map<string, MethodSpecificCodePtr>::iterator it;
  for (it = m_mapMethodsSpecificCode.begin(); it != m_mapMethodsSpecificCode.end(); it++)
  {
    v_names->push_back((*it).first);
  }

  return v_names;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int ClassSpecificCode::GetMethodCount()
{
  return (unsigned int) m_mapMethodsSpecificCode.size();
}

////////////////////////////////////////////////////////////////////////////////
