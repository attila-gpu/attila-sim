////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class EnumDescription;
    class StructDescription;
    class ClassDescription;
  }

  namespace Utilities
  {
    class Debug
    {
    public:
      
      static void Print(std::vector<Items::EnumDescription>& enums);
      static void Print(std::vector<Items::StructDescription>& structs);
      static void Print(std::vector<Items::ClassDescription>& classes);
      static void Print(regex::match_results& results);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
