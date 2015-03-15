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
  namespace Config
  {
    class ParserConfiguration;
  }
  
  namespace Items
  {
    class ClassDescription;
    class MethodDescription;
  }
  
  namespace Parser
  {
    class ClassExtractor : public IExtractor
    {
    public:

      ClassExtractor(Config::ParserConfiguration& config, std::string& cadena);
      ~ClassExtractor();

      std::vector<Items::ClassDescription>& GetClasses();

    protected:

      Config::ParserConfiguration& m_config;
      std::vector<Items::ClassDescription>* m_lstClasses;

      MatchResults Match(std::string& cadena);
      void Parse(const MatchResults& resultat);

      void ParseClass(const MatchResults& resultat);
      void ParseClassMethods(const std::string& name, const std::string& cadena);
      bool ParseClassMethod(Items::ClassDescription& cdes, std::string& cadena);
      bool ParseClassMethodParams(Items::ClassDescription& cdes, Items::MethodDescription& cmethod, std::string& cadena);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////