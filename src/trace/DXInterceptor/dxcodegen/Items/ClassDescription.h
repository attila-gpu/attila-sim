////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/MethodDescription.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class ClassDescription : public SmartPointer
    {
    public:
      
      ClassDescription();
      virtual ~ClassDescription();
      
      void SetName(const std::string& name);
      std::string& GetName();

      void AddMethod(const MethodDescriptionPtr method);
      unsigned int GetMethodsCount();
      MethodDescriptionPtr GetMethod(unsigned int position);
      MethodDescriptionPtr GetMethod(const std::string& name);

    protected:

      std::string m_name;
      std::vector<MethodDescriptionPtr> m_lstMethods;
      std::map<std::string, MethodDescriptionPtr> m_mapMethods;

    };

    ////////////////////////////////////////////////////////////////////////////

    typedef smart_ptr<ClassDescription> ClassDescriptionPtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
