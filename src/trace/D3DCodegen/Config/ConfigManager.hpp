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

      ConfigManager(const std::string& filename, bool verbose=false, bool createIfNotExists=false);

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
