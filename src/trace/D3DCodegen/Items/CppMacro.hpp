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

#pragma once

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class CppMacro
    {
    public:

      CppMacro(const std::string& left, const std::string& right);

      inline std::string& Left(void) {return m_left;}
      inline std::string& Right(void) {return m_right;}

    protected:

      std::string m_left;
      std::string m_right;

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
