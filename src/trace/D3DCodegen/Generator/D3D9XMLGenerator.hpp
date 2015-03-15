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
    class GeneratorConfiguration;
  }
  
  namespace Items
  {
    class ClassDescription;
    class StructDescription;
    class EnumDescription;
    class MethodDescription;
  }

  namespace Parser
  {
      class DXHParser;
  }

  namespace Generator
  {
    class D3D9XMLGenerator: public IGenerator
    {
    public:

      /*******************
      Accessors
      *******************/
      void setClasses(std::vector<Items::ClassDescription> &c);
      void setStructs(std::vector<Items::StructDescription> &s);
      void setEnums(std::vector<Items::EnumDescription> &e);

      
      /*******************
      Generates a XML representation of D3D9 Api
      ********************/
      void GenerateCode();

      D3D9XMLGenerator();

    private:
        /**************************
        Generate global functions, because
        DXHParser doesn't parse them.
        **************************/
        void generateFunctions(std::ostream *o);
        void generateMethod(std::ostream *o, Items::MethodDescription &m);
        void generateClass(std::ostream *o, Items::ClassDescription &c);
        void generateEnum(std::ostream *o, Items::EnumDescription &e);
        void generateStruct(std::ostream *o, Items::StructDescription &s);
        std::vector<Items::ClassDescription> classes;
        std::map<std::string, std::string> inheritance;
        std::vector<Items::StructDescription> structs;
        std::vector<Items::EnumDescription> enums;
    };
  }
}

////////////////////////////////////////////////////////////////////////////////
