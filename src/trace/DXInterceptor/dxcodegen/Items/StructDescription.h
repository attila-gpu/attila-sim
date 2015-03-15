////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/StructDescriptionField.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class StructDescription : public SmartPointer
    {
    public:

      StructDescription();
      virtual ~StructDescription();
      
      void SetName(const std::string& name);
      std::string& GetName();

      void AddField(const StructDescriptionFieldPtr field);
      unsigned int GetFieldCount();
      StructDescriptionFieldPtr GetField(unsigned int position);

    protected:

      std::string m_name;
      std::vector<StructDescriptionFieldPtr> m_lstFields;
      std::map<std::string, std::string> m_mapFields;
    
    };

    ////////////////////////////////////////////////////////////////////////////
    
    typedef smart_ptr<StructDescription> StructDescriptionPtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
