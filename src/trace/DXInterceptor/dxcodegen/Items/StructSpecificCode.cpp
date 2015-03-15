////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items/StructFieldSpecificCode.h"
#include "Items/StructSpecificCode.h"

using namespace std;
using namespace dxcodegen::Items;

////////////////////////////////////////////////////////////////////////////////

StructSpecificCode::StructSpecificCode(const string& name) :
m_name(name)
{
}

////////////////////////////////////////////////////////////////////////////////

StructSpecificCode::~StructSpecificCode()
{
}

////////////////////////////////////////////////////////////////////////////////

const string& StructSpecificCode::GetName() const
{
  return m_name;
}

////////////////////////////////////////////////////////////////////////////////

void StructSpecificCode::AddField(StructFieldSpecificCodePtr field)
{
  m_mapStructFieldsSpecificCode[field->GetName()] = field;
}

////////////////////////////////////////////////////////////////////////////////

StructFieldSpecificCodePtr StructSpecificCode::GetField(const string& name)
{
  map<string, StructFieldSpecificCodePtr>::iterator field = m_mapStructFieldsSpecificCode.find(name);
  if (field != m_mapStructFieldsSpecificCode.end())
  {
    return (*field).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

vector<string>* StructSpecificCode::GetFieldNames()
{
  vector<string>* v_names = new vector<string>();

  map<string, StructFieldSpecificCodePtr>::iterator it;
  for (it = m_mapStructFieldsSpecificCode.begin(); it != m_mapStructFieldsSpecificCode.end(); it++)
  {
    v_names->push_back((*it).first);
  }

  return v_names;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int StructSpecificCode::GetFieldCount() const
{
  return (unsigned int) m_mapStructFieldsSpecificCode.size();
}

////////////////////////////////////////////////////////////////////////////////
