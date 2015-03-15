////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Items/AttributeSpecificCode.h"
#include "Items/MethodSpecificCode.h"
#include "Items/NewMethodSpecificCode.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class ClassSpecificCode : public SmartPointer
    {
    public:

      ClassSpecificCode(const std::string& name);
      virtual ~ClassSpecificCode();

      std::string& GetName();

      void AddWrapperAttribute(AttributeSpecificCodePtr attrib);
      AttributeSpecificCodePtr GetWrapperAttribute(const std::string& name);
      std::vector<std::string>* GetWrapperAttributeNames();
      unsigned int GetWrapperAttributeCount();

      void AddStubAttribute(AttributeSpecificCodePtr attrib);
      AttributeSpecificCodePtr GetStubAttribute(const std::string& name);
      std::vector<std::string>* GetStubAttributeNames();
      unsigned int GetStubAttributeCount(); 
      
      void AddWrapperNewMethod(NewMethodSpecificCodePtr method);
      NewMethodSpecificCodePtr GetWrapperNewMethod(const std::string& name);
      std::vector<std::string>* GetWrapperNewMethodNames();
      unsigned int GetWrapperNewMethodCount();

      void AddStubNewMethod(NewMethodSpecificCodePtr method);
      NewMethodSpecificCodePtr GetStubNewMethod(const std::string& name);
      std::vector<std::string>* GetStubNewMethodNames();
      unsigned int GetStubNewMethodCount();
      
      void AddMethod(MethodSpecificCodePtr method);
      MethodSpecificCodePtr GetMethod(const std::string& name);
      std::vector<std::string>* GetMethodNames();
      unsigned int GetMethodCount();

    protected:

      std::string m_name;
      std::map<std::string, AttributeSpecificCodePtr> m_mapWrapperAttributesSpecificCode;
      std::map<std::string, AttributeSpecificCodePtr> m_mapStubAttributesSpecificCode;
      std::map<std::string, NewMethodSpecificCodePtr> m_mapWrapperNewMethodsSpecificCode;
      std::map<std::string, NewMethodSpecificCodePtr> m_mapStubNewMethodsSpecificCode;
      std::map<std::string, MethodSpecificCodePtr> m_mapMethodsSpecificCode;

    };

    ////////////////////////////////////////////////////////////////////////////

    typedef smart_ptr<ClassSpecificCode> ClassSpecificCodePtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
