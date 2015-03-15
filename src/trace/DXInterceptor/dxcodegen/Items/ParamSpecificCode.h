////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/SmartPointer.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class ParamSpecificCode : public SmartPointer
    {
    public:

      //////////////////////////////////////////////////////////////////////////
      
      enum ParamPolicyType
      {
        NoCapture = 1,
        ChangeSaveType,
        SaveSpecific,
        HelpParams
      };
      
      //////////////////////////////////////////////////////////////////////////
      
      struct ParamPolicy : public SmartPointer
      {
      public:

        ParamPolicy(ParamPolicyType type, const std::string& name)
        {
          m_type = type;
          m_name = name;
        }

        ParamPolicyType GetType() const
        {
          return m_type;
        }

        std::string GetName() const
        {
          return m_name;
        }

      protected:

        ParamPolicyType m_type;
        std::string m_name;

      };

      //////////////////////////////////////////////////////////////////////////
      
      typedef smart_ptr<ParamPolicy> ParamPolicyPtr;

      //////////////////////////////////////////////////////////////////////////
      
      struct NoCapturePolicy : public ParamPolicy
      {
      public:

        NoCapturePolicy() :
        ParamPolicy(NoCapture, "NoCapture")
        {
        }

      };

      //////////////////////////////////////////////////////////////////////////

      struct ChangeSaveTypePolicy : public ParamPolicy
      {
      public:

        ChangeSaveTypePolicy(const std::string& saveType) :
        ParamPolicy(ChangeSaveType, "ChangeSaveType"),
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
      
      struct SaveSpecificPolicy : public ParamPolicy
      {
      public:

        SaveSpecificPolicy() :
        ParamPolicy(SaveSpecific, "SaveSpecific")
        {
        }

      };
      
      //////////////////////////////////////////////////////////////////////////

      struct HelpParamsPolicy : public ParamPolicy
      {
      public:

        HelpParamsPolicy(const std::string& listHelpParams) :
        ParamPolicy(HelpParams, "HelpParams")
        {
          static const regex::rpattern patro_divisio("\\s* , \\s*", regex::EXTENDED);
          regex::split_results results;
          if (patro_divisio.split(listHelpParams, results) > 0)
          {
            for (regex::split_results::iterator it=results.begin(); it != results.end(); it++)
            {
              m_listHelpParams.push_back(atoi((*it).c_str()));
            }
          }
        }

        std::vector<unsigned int>& GetHelpParamList()
        {
          return m_listHelpParams;
        }

      protected:

        std::vector<unsigned int> m_listHelpParams;

      };

      //////////////////////////////////////////////////////////////////////////      
      
      ParamSpecificCode(unsigned int position);
      virtual ~ParamSpecificCode();
      
      unsigned int GetPosition() const;
      
      ParamPolicyPtr GetPolicy(ParamPolicyType type);
      ParamPolicyPtr GetPolicy(unsigned int position);
      void AddPolicy(ParamPolicyPtr policy);
      unsigned int GetPolicyCount() const;

    protected:

      unsigned int m_position;
      std::vector<ParamPolicyPtr> m_policies;

    };

    ////////////////////////////////////////////////////////////////////////////
    
    typedef smart_ptr<ParamSpecificCode> ParamSpecificCodePtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
