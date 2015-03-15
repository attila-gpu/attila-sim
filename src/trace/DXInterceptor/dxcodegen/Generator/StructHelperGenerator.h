////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Generator/IGenerator.h"
#include "Config/GeneratorConfiguration.h"
#include "Items/StructSpecificCode.h"
#include "Items/StructDescription.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Generator
  {
    class StructHelperGenerator : public IGenerator
    {
    public:

      StructHelperGenerator(Config::GeneratorConfiguration& config, const std::vector<Items::StructDescriptionPtr>& structures, const std::string& outputPath);
      virtual ~StructHelperGenerator();

      void GenerateCode();

    protected:

      Config::GeneratorConfiguration& m_config;
      std::vector<Items::StructDescriptionPtr> m_structures;
      std::string m_outputPath;
      std::string m_className;
      
      void GenerateHpp();
      void GenerateCpp();

      void GenerateDefinition(std::ofstream& sortida);
      void GenerateDefinitionMethods(std::ofstream& sortida);
      
      void GenerateImplementation(std::ofstream& sortida);
      void GenerateImplementationMethod(std::ofstream& sortida, Items::StructDescriptionPtr structure);
    
    };
  }
}

////////////////////////////////////////////////////////////////////////////////
