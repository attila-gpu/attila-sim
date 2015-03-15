////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Parser/IExtractor.h"
#include "Items/ClassDescription.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Config
  {
    class ParserConfiguration;
  }
  
  namespace Parser
  {
    class ClassExtractor : public IExtractor
    {
    public:

      ClassExtractor(Config::ParserConfiguration& config, std::string& cadena);
      virtual ~ClassExtractor();

      const std::vector<Items::ClassDescriptionPtr>& GetClasses();

    protected:

      Config::ParserConfiguration& m_config;
      std::vector<Items::ClassDescriptionPtr> m_lstClasses;

      MatchResults Match(std::string& cadena);
      void Parse(const MatchResults& resultat);

      void ParseClass(const MatchResults& resultat);
      void ParseClassMethods(const std::string& name, const std::string& cadena);
      bool ParseClassMethod(Items::ClassDescriptionPtr cdes, std::string& cadena);
      bool ParseClassMethodParams(Items::ClassDescriptionPtr cdes, Items::MethodDescriptionPtr cmethod, std::string& cadena);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////