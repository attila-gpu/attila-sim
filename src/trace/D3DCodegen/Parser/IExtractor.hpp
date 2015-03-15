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
  namespace Parser
  {
    class IExtractor
    {
    public:

      IExtractor(std::string& cadena, bool verbose=false);

      void Extract();

    protected:
    
      struct MatchResults
      {
        bool matched;
        int type;
        std::string text;

        MatchResults() :
        matched(false),
        type(0)
        {
        }
        
        void Clear()
        {
          matched = false;
          type = 0;
          text.clear();
        }
      };
      
      std::string& m_cadena;
      bool m_verbose;

      virtual MatchResults Match(std::string& cadena) = 0;
      virtual void Parse(const MatchResults& resultat) = 0;

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
