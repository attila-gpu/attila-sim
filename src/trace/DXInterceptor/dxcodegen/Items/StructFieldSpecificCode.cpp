////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items/StructFieldSpecificCode.h"

using namespace std;
using namespace dxcodegen::Items;

////////////////////////////////////////////////////////////////////////////////

StructFieldSpecificCode::StructFieldSpecificCode(const std::string& name) :
m_name(name)
{
}

////////////////////////////////////////////////////////////////////////////////

StructFieldSpecificCode::~StructFieldSpecificCode()
{
}

////////////////////////////////////////////////////////////////////////////////

const string& StructFieldSpecificCode::GetName() const
{
  return m_name;
}

////////////////////////////////////////////////////////////////////////////////

StructFieldSpecificCode::StructFieldPolicyPtr StructFieldSpecificCode::GetPolicy(StructFieldPolicyType type)
{
  StructFieldPolicyPtr policy = NULL;
  for (vector<StructFieldPolicyPtr>::iterator it=m_policies.begin(); it != m_policies.end(); it++)
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

StructFieldSpecificCode::StructFieldPolicyPtr StructFieldSpecificCode::GetPolicy(unsigned int position)
{
  StructFieldPolicyPtr policy = NULL;
  if (position < (unsigned int) m_policies.size())
  {
    policy = m_policies[position];
  }

  return policy;
}

////////////////////////////////////////////////////////////////////////////////

void StructFieldSpecificCode::AddPolicy(StructFieldSpecificCode::StructFieldPolicyPtr policy)
{
  bool found = false;
  for (vector<StructFieldPolicyPtr>::iterator it=m_policies.begin(); it != m_policies.end(); it++)
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

unsigned int StructFieldSpecificCode::GetPolicyCount() const
{
  return (unsigned int) m_policies.size();
}

////////////////////////////////////////////////////////////////////////////////
