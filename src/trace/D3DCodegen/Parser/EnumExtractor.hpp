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
    class EnumDescription;
  }
  
  namespace Parser
  {
    class EnumExtractor : public IExtractor
    {
    public:

      EnumExtractor(Config::ParserConfiguration& config, std::string& cadena);
      virtual ~EnumExtractor();
      
      std::vector<Items::EnumDescription>& GetEnums();      

    protected:

      Config::ParserConfiguration& m_config;
      std::vector<Items::EnumDescription>* m_lstEnums;

      MatchResults Match(std::string& cadena);
      void Parse(const MatchResults& resultat);
      
      MatchResults MatchTypedefEnum(std::string& cadena);
      MatchResults MatchEnum(std::string& cadena);

      void ParseTypedefEnum(const MatchResults& resultat);
      void ParseEnum(const MatchResults& resultat);
      void ParseEnumMembers(const std::string& name, const std::string& cadena);
      bool ParseEnumMember(Items::EnumDescription& edes, std::string& cadena);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////