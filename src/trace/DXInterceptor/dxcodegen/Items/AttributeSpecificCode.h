////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Items/SmartPointer.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class AttributeSpecificCode : public SmartPointer
    {
    public:

      AttributeSpecificCode(const std::string& name, const std::string& type);
      virtual ~AttributeSpecificCode();
      
      std::string GetName() const;
      std::string GetType() const;

    protected:

      std::string m_name;
      std::string m_type;

    };

    ////////////////////////////////////////////////////////////////////////////
    
    typedef smart_ptr<AttributeSpecificCode> AttributeSpecificCodePtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
