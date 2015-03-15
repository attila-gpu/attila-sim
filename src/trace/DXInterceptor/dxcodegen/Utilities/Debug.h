////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/EnumDescription.h"
#include "Items/StructDescription.h"
#include "Items/ClassDescription.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Utilities
  {
    class Debug
    {
    public:
      
      static void Print(std::vector<Items::EnumDescriptionPtr>& enums);
      static void Print(std::vector<Items::StructDescriptionPtr>& structs);
      static void Print(std::vector<Items::ClassDescriptionPtr>& classes);
      static void Print(regex::match_results& results);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
