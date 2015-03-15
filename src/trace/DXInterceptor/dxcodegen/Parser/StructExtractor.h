////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Parser/IExtractor.h"
#include "Items/StructDescription.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Config
  {
    class ParserConfiguration;
  }
  
  namespace Parser
  {
    class StructExtractor : public IExtractor
    {
    public:

      StructExtractor(Config::ParserConfiguration& config, std::string& cadena);
      virtual ~StructExtractor();

      const std::vector<Items::StructDescriptionPtr>& GetStructs();

    protected:

      Config::ParserConfiguration& m_config;
      std::vector<Items::StructDescriptionPtr> m_lstStructs;
      
      MatchResults Match(std::string& cadena);
      void Parse(const MatchResults& resultat);

      MatchResults MatchTypedefStruct(std::string& cadena);
      MatchResults MatchStruct(std::string& cadena);

      void ParseTypedefStruct(const MatchResults& resultat);
      void ParseStruct(const MatchResults& resultat);
      void ParseStructMembers(const std::string& name, const std::string& cadena);
      bool ParseStructMember(Items::StructDescriptionPtr sdes, std::string& cadena);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////