////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Parser
  {
    class IExtractor
    {
    public:

      IExtractor(std::string& cadena, bool verbose=false);
      virtual ~IExtractor();

      void Extract();

    protected:
    
      struct MatchResults
      {
        bool matched;
        int type;
        std::string text;

        MatchResults() :
        matched(false),
        type(0)
        {
        }
        
        void Clear()
        {
          matched = false;
          type = 0;
          text.clear();
        }
      };
      
      std::string& m_cadena;
      bool m_verbose;

      virtual MatchResults Match(std::string& cadena) = 0;
      virtual void Parse(const MatchResults& resultat) = 0;

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
