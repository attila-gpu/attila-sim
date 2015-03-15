////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Items/TypeSpecificCode.h"
#include "Items/StructSpecificCode.h"
#include "Items/ClassSpecificCode.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Config
  {
    class GeneratorConfiguration
    {
    public:

      GeneratorConfiguration();
      virtual ~GeneratorConfiguration();
      
      void Clear();
      
      void SetOutputPath(const std::string& path);
      std::string& GetOutputPath();

      void SetWrapperSuffix(const std::string& wrapperSuffix);
      std::string& GetWrapperSuffix();

      void SetStubSuffix(const std::string& stubSuffix);
      std::string& GetStubSuffix();

      void SetWrapperBaseClass(const std::string& wrapperBaseClass);
      std::string& GetWrapperBaseClass();

      void SetStubBaseClass(const std::string& stubBaseClass);
      std::string& GetStubBaseClass();

      void AddTypeSpecificCode(Items::TypeSpecificCodePtr tipo);
      Items::TypeSpecificCodePtr GetTypeSpecificCode(const std::string& name);

      void AddStructSpecificCode(Items::StructSpecificCodePtr estructura);
      Items::StructSpecificCodePtr GetStructSpecificCode(const std::string& name);
      
      void AddClassSpecificCode(Items::ClassSpecificCodePtr classe);
      Items::ClassSpecificCodePtr GetClassSpecificCode(const std::string& name);

    protected:

      std::string m_outputPath;
      std::string m_wrapperSuffix;
      std::string m_stubSuffix;
      std::string m_wrapperBaseClass;
      std::string m_stubBaseClass;
      
      std::map<std::string, Items::TypeSpecificCodePtr> m_mapTypesSpecificCode;
      std::map<std::string, Items::StructSpecificCodePtr> m_mapStructsSpecificCode;
      std::map<std::string, Items::ClassSpecificCodePtr> m_mapClassesSpecificCode;

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
