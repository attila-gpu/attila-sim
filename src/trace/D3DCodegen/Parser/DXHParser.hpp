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
  namespace Config
  {
    class ParserConfiguration;
  }

  namespace Items
  {
    class CppMacro;
    class EnumDescription;
    class StructDescription;
    class ClassDescription;
  }

  namespace Parser
  {
    class DXHParser
    {
    public:

      DXHParser(Config::ParserConfiguration& config, bool verbose=false);
      virtual ~DXHParser();

      void ParseFiles();

      std::vector<Items::ClassDescription>& GetClasses();
      std::vector<Items::StructDescription>& GetStructs();
      std::vector<Items::EnumDescription>& GetEnums();

    protected:

      Config::ParserConfiguration& m_config;
      bool m_verbose;
      
      std::vector<Items::EnumDescription>* m_lstEnums;
      std::vector<Items::StructDescription>* m_lstStructs;
      std::vector<Items::ClassDescription>* m_lstClasses;
      
      void ParseFile(const std::string& filename);
      void ReadFileData(const std::string& filename, std::string& cadena);

      void AddEnums(std::vector<Items::EnumDescription>& enums);
      void AddStructs(std::vector<Items::StructDescription>& structs);
      void AddClasses(std::vector<Items::ClassDescription>& classes);
    };
  }  
}

////////////////////////////////////////////////////////////////////////////////
