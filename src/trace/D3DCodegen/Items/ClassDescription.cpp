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
#include "Items/ClassDescription.hpp"

using namespace std;
using namespace dxcodegen::Items;

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

void ClassDescription::AddMethod(const MethodDescription& method)
{
  m_lstMethods.push_back(method);
}

////////////////////////////////////////////////////////////////////////////////

unsigned int ClassDescription::GetMethodsCount()
{
  return (unsigned int) m_lstMethods.size();
}

////////////////////////////////////////////////////////////////////////////////

MethodDescription& ClassDescription::GetMethod(unsigned int position)
{
  static MethodDescription method;
  if (m_lstMethods.size() && position < m_lstMethods.size())
  {
    return m_lstMethods[position];
  }
  return method;
}

////////////////////////////////////////////////////////////////////////////////
