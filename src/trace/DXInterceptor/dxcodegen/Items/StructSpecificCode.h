////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/StructFieldSpecificCode.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class StructSpecificCode : public SmartPointer
    {
    public:

      StructSpecificCode(const std::string& name);
      virtual ~StructSpecificCode();
      
      const std::string& GetName() const;
      
      void AddField(StructFieldSpecificCodePtr field);
      StructFieldSpecificCodePtr GetField(const std::string& name);
      std::vector<std::string>* GetFieldNames();
      unsigned int GetFieldCount() const;

    protected:

      std::string m_name;
      std::map<std::string, StructFieldSpecificCodePtr> m_mapStructFieldsSpecificCode;

    };

    ////////////////////////////////////////////////////////////////////////////
    
    typedef smart_ptr<StructSpecificCode> StructSpecificCodePtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
