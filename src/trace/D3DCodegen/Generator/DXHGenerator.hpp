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
  namespace Config
  {
    class GeneratorConfiguration;
  }
  
  namespace Items
  {
    class ClassDescription;
  }

  namespace Generator
  {
    class IGenerator;
    class ClassGenerator;

    class DXHGenerator
    {
    public:
      
      DXHGenerator(Config::GeneratorConfiguration& config);
      ~DXHGenerator();

      void AddClasses(std::vector<Items::ClassDescription>& classes);
      void GenerateCode();
    
    protected:

      Config::GeneratorConfiguration& m_config;
      std::string m_pathGeneration;
      std::vector<IGenerator*>* m_lstGenerators;

      void CreateGenerationPath();
      void SetGenerationPath(const std::string& path);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
