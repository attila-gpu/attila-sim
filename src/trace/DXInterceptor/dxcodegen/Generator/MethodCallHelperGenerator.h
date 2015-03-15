////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Generator/IGenerator.h"
#include "Items/ClassDescription.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Generator
  {
    class MethodCallHelperGenerator : public IGenerator
    {
    public:

      MethodCallHelperGenerator(const std::vector<Items::ClassDescriptionPtr>& classes, const std::string& outputPath);
      virtual ~MethodCallHelperGenerator();

      void GenerateCode();

    protected:

      std::vector<Items::ClassDescriptionPtr> m_classes;
      
      std::string m_outputPath;
      std::string m_className;
      
      void GenerateHpp();
      void GenerateCpp();

      void GenerateDefinition(std::ofstream& sortida);
      void GenerateMethodCallTokens(std::ofstream& sortida);
      void GenerateHelperMethodsHeaders(std::ofstream& sortida);

      void GenerateImplementation(std::ofstream& sortida);
      void GenerateImplementationGetClassName(std::ofstream& sortida);
      void GenerateImplementationGetMethodName(std::ofstream& sortida);
    
    };
  }
}

////////////////////////////////////////////////////////////////////////////////
