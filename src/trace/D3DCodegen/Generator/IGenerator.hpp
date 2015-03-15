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
  namespace Generator
  {
    class IGenerator
    {
    public:

      void SetHeaderComment(const std::string& message);
      std::string& GetHeaderCommment();
      
      virtual void GenerateCode() = 0;

    protected:

      std::string m_headerComment;

      std::ofstream* CreateFilename(const std::string& filename);
      void WriteHeaderComment(std::ofstream* of);
      void CloseFilename(std::ofstream* of);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
