////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Generator/IGenerator.h"
#include "Config/GeneratorConfiguration.h"
#include "Items/ClassSpecificCode.h"
#include "Items/ClassDescription.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Generator
  {
    class ClassStubGenerator : public IGenerator
    {
    public:

      ClassStubGenerator(Config::GeneratorConfiguration& config, Items::ClassDescriptionPtr cdes, const std::string& outputPath, const std::string& classNameSuffix, const std::string& baseClassName);
      virtual ~ClassStubGenerator();

      void GenerateCode();

    protected:

      Config::GeneratorConfiguration& m_config;
      Items::ClassDescriptionPtr m_classDescription;
      Items::ClassSpecificCodePtr m_classSpecificCode;
      std::string m_outputPath;
      std::string m_className;
      std::string m_baseClassName;

      void GenerateHpp();
      void GenerateCpp();

      void GenerateDefinition(std::ofstream& sortida);
      void GenerateDefinitionMethods(std::ofstream& sortida);
      void GenerateDefinitionSpecificCode(std::ofstream& sortida);
      void GenerateDefinitionSpecificCodeAttributes(std::ofstream& sortida);
      void GenerateDefinitionSpecificCodeMethodAdd(std::ofstream& sortida);
      
      void GenerateImplementation(std::ofstream& sortida);
      void GenerateImplementationHandleCall(std::ofstream& sortida);
      void GenerateImplementationHandleCallToken(std::ofstream& sortida, Items::MethodDescriptionPtr method);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
