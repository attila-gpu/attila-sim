////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items/ParamSpecificCode.h"
#include "Items/MethodSpecificCode.h"

using namespace std;
using namespace dxcodegen::Items;

////////////////////////////////////////////////////////////////////////////////

MethodSpecificCode::MethodSpecificCode(const string& name) :
m_name(name),
m_policy(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

MethodSpecificCode::~MethodSpecificCode()
{
}

////////////////////////////////////////////////////////////////////////////////

const string& MethodSpecificCode::GetName() const
{
  return m_name;
}

////////////////////////////////////////////////////////////////////////////////

MethodSpecificCode::MethodPolicyPtr MethodSpecificCode::GetPolicy() const
{
  return m_policy;
}

////////////////////////////////////////////////////////////////////////////////

void MethodSpecificCode::SetPolicy(MethodSpecificCode::MethodPolicyPtr policy)
{
  m_policy = policy;
}

////////////////////////////////////////////////////////////////////////////////

void MethodSpecificCode::AddParam(ParamSpecificCodePtr param)
{
  m_mapParamsSpecificCode[param->GetPosition()] = param;
}

////////////////////////////////////////////////////////////////////////////////

ParamSpecificCodePtr MethodSpecificCode::GetParam(unsigned int position)
{
  map<unsigned int, ParamSpecificCodePtr>::iterator param = m_mapParamsSpecificCode.find(position);
  if (param != m_mapParamsSpecificCode.end())
  {
    return (*param).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

std::vector<unsigned int>* MethodSpecificCode::GetParamPositions()
{
  vector<unsigned int>* v_positions = new vector<unsigned int>();

  map<unsigned int, ParamSpecificCodePtr>::iterator it;
  for (it = m_mapParamsSpecificCode.begin(); it != m_mapParamsSpecificCode.end(); it++)
  {
    v_positions->push_back((*it).first);
  }

  return v_positions;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int MethodSpecificCode::GetParamCount()
{
  return (unsigned int) m_mapParamsSpecificCode.size();
}

////////////////////////////////////////////////////////////////////////////////
