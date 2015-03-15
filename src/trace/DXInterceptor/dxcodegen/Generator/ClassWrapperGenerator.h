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
    class ClassWrapperGenerator : public IGenerator
    {
    public:

      ClassWrapperGenerator(Config::GeneratorConfiguration& config, Items::ClassDescriptionPtr cdes, const std::string& outputPath, const std::string& classNameSuffix, const std::string& baseClassName);
      virtual ~ClassWrapperGenerator();

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
      void GenerateMethodHeader(std::ofstream& sortida, Items::MethodDescriptionPtr method, const std::string& altname = "", bool passMethodCallSaverInstance = false);
      void GenerateMethodCall(std::ofstream& sortida, Items::MethodDescriptionPtr method, const std::string& altname = "", bool passMethodCallSaverInstance = false);

      void GenerateDefinition(std::ofstream& sortida);
      void GenerateDefinitionMethods(std::ofstream& sortida);
      void GenerateDefinitionMethod(std::ofstream& sortida, Items::MethodDescriptionPtr method);
      void GenerateDefinitionSpecificCode(std::ofstream& sortida);
      void GenerateDefinitionSpecificCodeAttributes(std::ofstream& sortida);
      void GenerateDefinitionSpecificCodeMethodAdd(std::ofstream& sortida);
      void GenerateDefinitionSpecificCodeMethodReplace(std::ofstream& sortida);
      
      void GenerateImplementation(std::ofstream& sortida);
      void GenerateImplementationMethods(std::ofstream& sortida);
      void GenerateImplementationMethod(std::ofstream& sortida, Items::MethodDescriptionPtr method);
      void GenerateImplementationMethodBodyCall(std::ofstream& sortida, Items::MethodDescriptionPtr method);
      void GenerateImplementationMethodBodySaveCall(std::ofstream& sortida, Items::MethodDescriptionPtr method);
      bool GenerateImplementationMethodBodySaveCallParam(std::ofstream& sortida, Items::MethodDescriptionPtr method, unsigned int paramPosition);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
