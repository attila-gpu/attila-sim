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

#include "Items/MethodDescription.hpp"

using namespace std;
using namespace dxcodegen::Items;

////////////////////////////////////////////////////////////////////////////////

void MethodDescription::SetType(const string& type)
{
  m_type = type;
}

////////////////////////////////////////////////////////////////////////////////

string& MethodDescription::GetType()
{
  return m_type;
}

////////////////////////////////////////////////////////////////////////////////

void MethodDescription::SetName(const string& name)
{
  m_name = name;
}

////////////////////////////////////////////////////////////////////////////////

string& MethodDescription::GetName()
{
  return m_name;
}

////////////////////////////////////////////////////////////////////////////////

void MethodDescription::AddParam(const MethodDescriptionParam& param)
{
  m_lstParams.push_back(param);
}

////////////////////////////////////////////////////////////////////////////////

unsigned int MethodDescription::GetParamsCount()
{
  return (unsigned int) m_lstParams.size();
}

////////////////////////////////////////////////////////////////////////////////

MethodDescriptionParam& MethodDescription::GetParam(unsigned int position)
{
  static MethodDescriptionParam param;
  if (m_lstParams.size() && position < m_lstParams.size())
  {
    return m_lstParams[position];
  }
  return param;
}

////////////////////////////////////////////////////////////////////////////////
