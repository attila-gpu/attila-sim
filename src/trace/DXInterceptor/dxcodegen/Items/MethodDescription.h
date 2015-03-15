////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "MethodDescriptionParam.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class MethodDescription : public SmartPointer
    {
    public:

      MethodDescription();
      virtual ~MethodDescription();
      
      void SetType(const std::string& type);
      std::string& GetType();

      void SetName(const std::string& name);
      std::string& GetName();

      void AddParam(const MethodDescriptionParamPtr param);
      unsigned int GetParamsCount();
      MethodDescriptionParamPtr GetParam(unsigned int position);

    protected:

      std::string m_type;
      std::string m_name;
      std::vector<MethodDescriptionParamPtr> m_lstParams;

    };

    ////////////////////////////////////////////////////////////////////////////

    typedef smart_ptr<MethodDescription> MethodDescriptionPtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
