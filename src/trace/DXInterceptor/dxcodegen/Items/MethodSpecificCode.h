////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/ParamSpecificCode.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class MethodSpecificCode : public SmartPointer
    {
    public:

      //////////////////////////////////////////////////////////////////////////
      
      enum MethodPolicyType
      {
        ReplaceBoth = 1,
        ReplaceWrapperOnly,
        ReplaceStubOnly
      };
      
      //////////////////////////////////////////////////////////////////////////
      
      struct MethodPolicy : public SmartPointer
      {
      public:

        MethodPolicy(MethodPolicyType type, const std::string& name)
        {
          m_type = type;
          m_name = name;
        }

        MethodPolicyType GetType() const
        {
          return m_type;
        }

        std::string GetName() const
        {
          return m_name;
        }

      protected:

        MethodPolicyType m_type;
        std::string m_name;

      };

      //////////////////////////////////////////////////////////////////////////
      
      typedef smart_ptr<MethodPolicy> MethodPolicyPtr;

      //////////////////////////////////////////////////////////////////////////
      
      struct ReplaceBothPolicy : public MethodPolicy
      {
      public:

        ReplaceBothPolicy(const std::string& methodName, bool passCallSaver) :
        MethodPolicy(ReplaceBoth, "ReplaceBoth"),
        m_methodName(methodName),
        m_passCallSaver(passCallSaver)
        {
        }

        std::string& GetMethodName()
        {
          return m_methodName;
        }

        bool GetPassCallSaver() const
        {
          return m_passCallSaver;
        }

      protected:

        std::string m_methodName;
        bool m_passCallSaver;

      };

      //////////////////////////////////////////////////////////////////////////

      struct ReplaceWrapperOnlyPolicy : public MethodPolicy
      {
      public:

        ReplaceWrapperOnlyPolicy(const std::string& methodName, bool passCallSaver) :
        MethodPolicy(ReplaceWrapperOnly, "ReplaceWrapperOnly"),
        m_methodName(methodName),
        m_passCallSaver(passCallSaver)
        {
        }

        std::string& GetMethodName()
        {
          return m_methodName;
        }

        bool GetPassCallSaver() const
        {
          return m_passCallSaver;
        }

      protected:

        std::string m_methodName;
        bool m_passCallSaver;

      };

      //////////////////////////////////////////////////////////////////////////

      struct ReplaceStubOnlyPolicy : public MethodPolicy
      {
      public:

        ReplaceStubOnlyPolicy() :
        MethodPolicy(ReplaceStubOnly, "ReplaceStubOnly")
        {
        }

      };

      //////////////////////////////////////////////////////////////////////////      
      
      MethodSpecificCode(const std::string& name);
      virtual ~MethodSpecificCode();
      
      const std::string& GetName() const;
      
      MethodPolicyPtr GetPolicy() const;
      void SetPolicy(MethodPolicyPtr policy);

      void AddParam(ParamSpecificCodePtr param);
      ParamSpecificCodePtr GetParam(unsigned int position);
      std::vector<unsigned int>* GetParamPositions();
      unsigned int GetParamCount();

    protected:

      std::string m_name;
      MethodPolicyPtr m_policy;
      std::map<unsigned int, ParamSpecificCodePtr> m_mapParamsSpecificCode;

    };

    ////////////////////////////////////////////////////////////////////////////
    
    typedef smart_ptr<MethodSpecificCode> MethodSpecificCodePtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
