////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items/ParamSpecificCode.h"

using namespace std;
using namespace dxcodegen::Items;

////////////////////////////////////////////////////////////////////////////////

ParamSpecificCode::ParamSpecificCode(unsigned int position) :
m_position(position)
{
}

////////////////////////////////////////////////////////////////////////////////

ParamSpecificCode::~ParamSpecificCode()
{
}

////////////////////////////////////////////////////////////////////////////////

unsigned int ParamSpecificCode::GetPosition() const
{
  return m_position;
}

////////////////////////////////////////////////////////////////////////////////

ParamSpecificCode::ParamPolicyPtr ParamSpecificCode::GetPolicy(ParamPolicyType type)
{
  ParamPolicyPtr policy = NULL;
  for (vector<ParamPolicyPtr>::iterator it=m_policies.begin(); it != m_policies.end(); it++)
  {
    if ((*it)->GetType() == type)
    {
      policy = *it;
      break;
    }
  }

  return policy;
}

////////////////////////////////////////////////////////////////////////////////

ParamSpecificCode::ParamPolicyPtr ParamSpecificCode::GetPolicy(unsigned int position)
{
  ParamPolicyPtr policy = NULL;
  if (position < (unsigned int) m_policies.size())
  {
    policy = m_policies[position];
  }

  return policy;
}

////////////////////////////////////////////////////////////////////////////////

void ParamSpecificCode::AddPolicy(ParamSpecificCode::ParamPolicyPtr policy)
{
  bool found = false;
  for (vector<ParamPolicyPtr>::iterator it=m_policies.begin(); it != m_policies.end(); it++)
  {
    if ((*it)->GetType() == policy->GetType())
    {
      found = true;
      break;
    }
  }
  
  if (!found)
  {
    m_policies.push_back(policy);
  }
}

////////////////////////////////////////////////////////////////////////////////

unsigned int ParamSpecificCode::GetPolicyCount() const
{
  return (unsigned int) m_policies.size();
}

////////////////////////////////////////////////////////////////////////////////
