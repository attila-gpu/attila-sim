////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/ClassSpecificCode.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Parser
  {
    class DXHParser;
  }

	namespace Config
  {
    class ParserConfiguration;
    class GeneratorConfiguration;

    class ConfigManager
    {
    public:

      ConfigManager(const std::string& filename, bool verbose=false, bool createIfNotExists=true);
      virtual ~ConfigManager();

      void GetParserConfiguration(ParserConfiguration& config);
      void GetGeneratorConfiguration(GeneratorConfiguration& config);

    protected:

      std::string m_filename;
      bool m_verbose;
      TiXmlDocument m_xmlDocument;

      void AddParserFiles(ParserConfiguration& config);
      void AddParserEnums(ParserConfiguration& config);
      void AddParserStructs(ParserConfiguration& config);
      void AddParserClasses(ParserConfiguration& config);
      void AddParserMacros(ParserConfiguration& config);
      
      void AddGeneratorAttributes(GeneratorConfiguration& config);
      void AddGeneratorTypesSpecificCode(GeneratorConfiguration& config);
      void AddGeneratorStructsSpecificCode(GeneratorConfiguration& config);
      void AddGeneratorClassesSpecificCode(GeneratorConfiguration& config);
      void AddGeneratorClassesSpecificCodeWrapperAttribsAdd(TiXmlNode* classNode, Items::ClassSpecificCodePtr classSC);
      void AddGeneratorClassesSpecificCodeStubAttribsAdd(TiXmlNode* classNode, Items::ClassSpecificCodePtr classSC);
      void AddGeneratorClassesSpecificCodeWrapperMethodsAdd(TiXmlNode* classNode, Items::ClassSpecificCodePtr classSC);
      void AddGeneratorClassesSpecificCodeStubMethodsAdd(TiXmlNode* classNode, Items::ClassSpecificCodePtr classSC);
      void AddGeneratorClassesSpecificCodeMethods(TiXmlNode* classNode, Items::ClassSpecificCodePtr classSC);
      
      bool AddHeader();
      bool AddSection(std::string sectionParent, std::string sectionName);
      bool AddSection(std::string sectionParent, std::string sectionName, std::string sectionText);
      bool AddSectionAttribute(std::string sectionName, std::string attribName, std::string attribValue);
      bool ExistsHeader();
      TiXmlNode* GetSection(std::string sectionName);
      std::vector<std::string>* GetTextList(std::string sectionName, std::string subSectionName);
      void Clear();
      bool FillDefault();
      std::string GetDirectXSDKPath();
      bool Save();

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
