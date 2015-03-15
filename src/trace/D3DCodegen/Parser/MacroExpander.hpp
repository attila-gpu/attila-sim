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
    class CppMacro;
  }

  namespace Parser
  {
    class MacroExpander
    {
    public:

      MacroExpander(std::vector<Items::CppMacro>& macros, bool verbose=false);

      void Expand(std::string& cadena);

    protected:

      std::vector<Items::CppMacro>& m_lstMacros;
      bool m_verbose;

      void MatchMacro(Items::CppMacro& macro, std::string& cadena);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
