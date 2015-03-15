////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/CppMacro.h"
#include "Items/EnumDescription.h"
#include "Items/StructDescription.h"
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
    class DXHParser
    {
    public:

      DXHParser(Config::ParserConfiguration& config, bool verbose=false);
      virtual ~DXHParser();

      void ParseFiles();

      const std::vector<Items::ClassDescriptionPtr>& GetClasses();
      const std::vector<Items::EnumDescriptionPtr>& GetEnumerations();
      const std::vector<Items::StructDescriptionPtr>& GetStructures();

    protected:

      Config::ParserConfiguration& m_config;
      bool m_verbose;
      
      std::vector<Items::ClassDescriptionPtr> m_lstClasses;
      std::vector<Items::EnumDescriptionPtr> m_lstEnums;
      std::vector<Items::StructDescriptionPtr> m_lstStructs;
      
      void ParseFile(const std::string& filename);
      void ReadFileData(const std::string& filename, std::string& cadena);

      void AddEnums(const std::vector<Items::EnumDescriptionPtr>& enums);
      void AddStructs(const std::vector<Items::StructDescriptionPtr>& structs);
      void AddClasses(const std::vector<Items::ClassDescriptionPtr>& classes);
    };
  }  
}

////////////////////////////////////////////////////////////////////////////////
