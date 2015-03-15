////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXCodeGenException.h"
#include "Utilities/FileSystem.h"
#include "Config/GeneratorConfiguration.h"
#include "Items/ClassSpecificCode.h"
#include "Items/ClassDescription.h"
#include "Items/EnumDescription.h"
#include "Generator/IGenerator.h"
#include "Generator/MethodCallHelperGenerator.h"
#include "Generator/ClassWrapperGenerator.h"
#include "Generator/ClassStubGenerator.h"
#include "Generator/EnumHelperGenerator.h"
#include "Generator/StructHelperGenerator.h"
#include "Generator/DXHGenerator.h"

using namespace std;
using namespace dxcodegen;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;
using namespace dxcodegen::Generator;

////////////////////////////////////////////////////////////////////////////////

DXHGenerator::DXHGenerator(GeneratorConfiguration& config) :
m_config(config)
{
  SetGenerationPath(m_config.GetOutputPath());
}

////////////////////////////////////////////////////////////////////////////////

DXHGenerator::~DXHGenerator()
{
}

////////////////////////////////////////////////////////////////////////////////

void DXHGenerator::AddClasses(const vector<ClassDescriptionPtr>& classes)
{
  MethodCallHelperGenerator* mhelp = new MethodCallHelperGenerator(classes, m_pathGeneration);
  m_lstGenerators.push_back(mhelp);
  
  for (vector<ClassDescriptionPtr>::const_iterator it = classes.begin(); it != classes.end(); it++)
  {
    ClassWrapperGenerator* cgenwrapper = new ClassWrapperGenerator(m_config, *it, m_pathGeneration, m_config.GetWrapperSuffix(), m_config.GetWrapperBaseClass());
    m_lstGenerators.push_back(cgenwrapper);
    
    ClassStubGenerator* cgenstub = new ClassStubGenerator(m_config, *it, m_pathGeneration, m_config.GetStubSuffix(), m_config.GetStubBaseClass());
    m_lstGenerators.push_back(cgenstub);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXHGenerator::AddEnumerations(const vector<EnumDescriptionPtr>& enumerations)
{
  EnumHelperGenerator* egen = new EnumHelperGenerator(enumerations, m_pathGeneration);
  m_lstGenerators.push_back(egen);
}

////////////////////////////////////////////////////////////////////////////////

void DXHGenerator::AddStructures(const vector<StructDescriptionPtr>& structures)
{
  StructHelperGenerator* sgen = new StructHelperGenerator(m_config, structures, m_pathGeneration);
  m_lstGenerators.push_back(sgen);
}

////////////////////////////////////////////////////////////////////////////////

void DXHGenerator::GenerateCode()
{
  CreateGenerationPath();

  for (vector<IGeneratorPtr>::iterator it = m_lstGenerators.begin(); it != m_lstGenerators.end(); it++)
  {
    (*it)->GenerateCode();
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXHGenerator::SetGenerationPath(const string& path)
{
  if (!path.empty() && path[1] == ':')
  {
    m_pathGeneration = path;
    Utilities::FileSystem::DirectoryAddBackslash(m_pathGeneration);
  }
  else
  {
    if (path.empty())
    {
      m_pathGeneration.clear();
      m_pathGeneration = ".\\";
    }
    else
    {
      m_pathGeneration = Utilities::FileSystem::GetCurrentDirectory();
      m_pathGeneration += path;
      Utilities::FileSystem::DirectoryAddBackslash(m_pathGeneration);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXHGenerator::CreateGenerationPath()
{
  if (!m_pathGeneration.empty() && !Utilities::FileSystem::DirectoryExists(m_pathGeneration))
  {
    cout << "Try to create directory '" << m_pathGeneration << "'..." << endl;
    if (Utilities::FileSystem::CreateDirectory(m_pathGeneration, true))
    {
      cout << "Directory created OK" << endl;
    }
    
    if (!Utilities::FileSystem::DirectoryExists(m_pathGeneration))
    {
      DXCodeGenException e("could'nt create path '" + m_pathGeneration + "'");
      throw e;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
