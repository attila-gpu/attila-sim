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
  namespace Items
  {
    struct StructDescriptionMember
    {
      std::string type;
      std::string name;
    };

    class StructDescription
    {
    public:

      void SetName(const std::string& name);
      std::string& GetName();

      bool AddMember(const StructDescriptionMember& member);
      unsigned int GetMemberCount();
      StructDescriptionMember& GetMember(unsigned int position);

    protected:

      std::string m_name;
      std::vector<StructDescriptionMember> m_lstMembers;
      std::map<std::string,std::string> m_mapMembers;
    
    };
  }
}

////////////////////////////////////////////////////////////////////////////////
