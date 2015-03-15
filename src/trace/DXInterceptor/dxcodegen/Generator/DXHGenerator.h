////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Generator/IGenerator.h"
#include "Items/ClassSpecificCode.h"
#include "Items/ClassDescription.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Config
  {
    class GeneratorConfiguration;
  }

  namespace Generator
  {
    class DXHGenerator
    {
    public:
      
      DXHGenerator(Config::GeneratorConfiguration& config);
      virtual ~DXHGenerator();

      void AddClasses(const std::vector<Items::ClassDescriptionPtr>& classes);
      void AddEnumerations(const std::vector<Items::EnumDescriptionPtr>& enumerations);
      void AddStructures(const std::vector<Items::StructDescriptionPtr>& structures);
      void GenerateCode();
    
    protected:

      Config::GeneratorConfiguration& m_config;
      std::string m_pathGeneration;
      std::vector<IGeneratorPtr> m_lstGenerators;

      void CreateGenerationPath();
      void SetGenerationPath(const std::string& path);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
