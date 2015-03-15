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
#include "Parser/IExtractor.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Parser
  {
    class CommentExtractor : public IExtractor
    {
    public:

      CommentExtractor(std::string& cadena);

    protected:

      MatchResults Match(std::string& cadena);
      void Parse(const MatchResults& resultat);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////