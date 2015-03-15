////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/EnumDescriptionMember.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class EnumDescription : public SmartPointer
    {
    public:

      EnumDescription();
      virtual ~EnumDescription();
      
      void SetName(const std::string& name);
      std::string& GetName();
      
      void AddMember(const EnumDescriptionMemberPtr name);
      unsigned int GetMembersCount();
      EnumDescriptionMemberPtr GetMember(unsigned int position);

    protected:

      std::string m_name;
      std::vector<EnumDescriptionMemberPtr> m_lstMembers;
      std::map<std::string, std::string> m_mapMembers;

    };

    ////////////////////////////////////////////////////////////////////////////

    typedef smart_ptr<EnumDescription> EnumDescriptionPtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
