////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/SmartPointer.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class StructFieldSpecificCode : public SmartPointer
    {
    public:

      //////////////////////////////////////////////////////////////////////////
      
      enum StructFieldPolicyType
      {
        ChangeSaveType
      };
      
      //////////////////////////////////////////////////////////////////////////
      
      struct StructFieldPolicy : public SmartPointer
      {
      public:

        StructFieldPolicy(StructFieldPolicyType type, const std::string& name)
        {
          m_type = type;
          m_name = name;
        }

        StructFieldPolicyType GetType() const
        {
          return m_type;
        }

        std::string GetName() const
        {
          return m_name;
        }

      protected:

        StructFieldPolicyType m_type;
        std::string m_name;

      };

      //////////////////////////////////////////////////////////////////////////
      
      typedef smart_ptr<StructFieldPolicy> StructFieldPolicyPtr;

      //////////////////////////////////////////////////////////////////////////

      struct StructFieldChangeSaveTypePolicy : public StructFieldPolicy
      {
      public:

        StructFieldChangeSaveTypePolicy(const std::string& saveType) :
        StructFieldPolicy(ChangeSaveType, "ChangeSaveType"),
        m_saveType(saveType)
        {
        }

        std::string GetSaveType() const
        {
          return m_saveType;
        }

      protected:

        std::string m_saveType;
      
      };

      //////////////////////////////////////////////////////////////////////////      
      
      StructFieldSpecificCode(const std::string& name);
      virtual ~StructFieldSpecificCode();
      
      const std::string& GetName() const;
      
      StructFieldPolicyPtr GetPolicy(StructFieldPolicyType type);
      StructFieldPolicyPtr GetPolicy(unsigned int position);
      void AddPolicy(StructFieldPolicyPtr policy);
      unsigned int GetPolicyCount() const;

    protected:

      std::string m_name;
      std::vector<StructFieldPolicyPtr> m_policies;

    };

    ////////////////////////////////////////////////////////////////////////////
    
    typedef smart_ptr<StructFieldSpecificCode> StructFieldSpecificCodePtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
