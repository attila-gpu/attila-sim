////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Parser/IExtractor.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Parser
  {
    class PreprocessorExtractor : public IExtractor
    {
    public:

      PreprocessorExtractor(std::string& cadena);
      virtual ~PreprocessorExtractor();

    protected:

      MatchResults Match(std::string& cadena);
      void Parse(const MatchResults& resultat);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
