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
    class StructDescription;
  }
  
  namespace Parser
  {
    class StructExtractor : public IExtractor
    {
    public:

      StructExtractor(Config::ParserConfiguration& config, std::string& cadena);
      virtual ~StructExtractor();

      std::vector<Items::StructDescription>& GetStructs();

    protected:

      Config::ParserConfiguration& m_config;
      std::vector<Items::StructDescription>* m_lstStructs;
      
      MatchResults Match(std::string& cadena);
      void Parse(const MatchResults& resultat);

      MatchResults MatchTypedefStruct(std::string& cadena);
      MatchResults MatchStruct(std::string& cadena);

      void ParseTypedefStruct(const MatchResults& resultat);
      void ParseStruct(const MatchResults& resultat);
      void ParseStructMembers(const std::string& name, const std::string& cadena);
      bool ParseStructMember(Items::StructDescription& sdes, std::string& cadena);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////