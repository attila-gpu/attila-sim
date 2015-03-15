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
#include "Generator/IGenerator.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class ClassDescription;
    class MethodDescription;
  }
  
  namespace Generator
  {
    class ClassGenerator : public IGenerator
    {
    public:

      ClassGenerator(Items::ClassDescription& cdes, const std::string& outputPath);
      
      void GenerateCode();

    protected:

      Items::ClassDescription m_classDescription;
      std::string m_outputPath;
      std::string m_classNameSuffix;

      std::string GetClassName();
      void GenerateHeaderComment(std::string& comment);
      std::string GetDateTimeNow();
      void GenerateHpp();
      void GenerateCpp();
      void GenerateClassBody(std::ofstream* of);
      void GenerateClassMethods(std::ofstream* of);
      void GenerateClassMethod(std::ofstream* of, unsigned int position);
      void GenerateClassMethodBody(std::ofstream* of, unsigned int position);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
