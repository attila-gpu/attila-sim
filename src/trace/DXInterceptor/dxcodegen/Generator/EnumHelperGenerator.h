////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Generator/IGenerator.h"
#include "Items/EnumDescription.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Generator
  {
    class EnumHelperGenerator : public IGenerator
    {
    public:

      EnumHelperGenerator(const std::vector<Items::EnumDescriptionPtr>& enumerations, const std::string& outputPath);
      virtual ~EnumHelperGenerator();

      void GenerateCode();

    protected:

      std::vector<Items::EnumDescriptionPtr> m_enumerations;
      
      std::string m_outputPath;
      std::string m_className;
      
      void GenerateHpp();
      void GenerateCpp();

      void GenerateDefinition(std::ofstream& sortida);
      void GenerateDefinitionMethods(std::ofstream& sortida);
      
      void GenerateImplementation(std::ofstream& sortida);
      void GenerateImplementationDictInstance(std::ofstream& sortida, Items::EnumDescriptionPtr enumeration);
      void GenerateImplementationDictInitialization(std::ofstream& sortida, Items::EnumDescriptionPtr enumeration);
      void GenerateImplementationDictMethods(std::ofstream& sortida, Items::EnumDescriptionPtr enumeration);
    
    };
  }
}

////////////////////////////////////////////////////////////////////////////////
