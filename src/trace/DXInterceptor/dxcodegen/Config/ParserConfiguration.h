////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/CppMacro.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Config
  {
    class ParserConfiguration
    {
    public:

      ParserConfiguration();
      virtual ~ParserConfiguration();
      
      void Clear();
      
      void AddFile(const std::string& filename);
      std::vector<std::string>& GetFiles();

      void AddEnum(const std::string& name, bool exclude);
      std::vector<std::string>& GetEnumsToInclude();
      std::vector<std::string>& GetEnumsToExclude();

      void AddStruct(const std::string& name, bool exclude);
      std::vector<std::string>& GetStructsToInclude();
      std::vector<std::string>& GetStructsToExclude();

      void AddClass(const std::string& name, bool exclude);
      std::vector<std::string>& GetClassesToInclude();
      std::vector<std::string>& GetClassesToExclude();

      void AddMacro(const Items::CppMacroPtr macro);
      std::vector<Items::CppMacroPtr>& GetMacros();

      bool IsEnumParseCandidate(const std::string& name);
      bool IsStructParseCandidate(const std::string& name);
      bool IsClassParseCandidate(const std::string& name);

    protected:

      std::vector<std::string> m_lstFiles;
      std::vector<std::string> m_lstEnumsInclude;
      std::vector<std::string> m_lstEnumsExclude;
      std::vector<std::string> m_lstStructsInclude;
      std::vector<std::string> m_lstStructsExclude;
      std::vector<std::string> m_lstClassesInclude;
      std::vector<std::string> m_lstClassesExclude;
      std::vector<Items::CppMacroPtr> m_lstMacros;

      std::map<std::string, std::string> m_mapEnumsInclude;
      std::map<std::string, std::string> m_mapEnumsExclude;
      std::map<std::string, std::string> m_mapStructsInclude;
      std::map<std::string, std::string> m_mapStructsExclude;
      std::map<std::string, std::string> m_mapClassesInclude;
      std::map<std::string, std::string> m_mapClassesExclude;

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
