////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Config/GeneratorConfiguration.h"
#include "Items/StructSpecificCode.h"
#include "Items/StructDescription.h"
#include "Generator/StructHelperGenerator.h"

using namespace std;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;
using namespace dxcodegen::Generator;

////////////////////////////////////////////////////////////////////////////////

StructHelperGenerator::StructHelperGenerator(GeneratorConfiguration& config, const std::vector<StructDescriptionPtr>& structures, const string& outputPath) :
m_config(config),
m_outputPath(outputPath),
m_className("DXStructHelper"),
IGenerator("class DXStructHelper")
{
  m_structures = structures;
}

////////////////////////////////////////////////////////////////////////////////

StructHelperGenerator::~StructHelperGenerator()
{
}

////////////////////////////////////////////////////////////////////////////////

void StructHelperGenerator::GenerateCode()
{
  GenerateHpp();
  GenerateCpp();
}

////////////////////////////////////////////////////////////////////////////////

void StructHelperGenerator::GenerateHpp()
{
  string filename = m_className + ".h";
  ofstream* sortida = CreateFilename(m_outputPath + filename);
  if (sortida && sortida->is_open())
  {
    cout << "Creating '" << filename << "'" << endl;
    GenerateDefinition(*sortida);
  }
  CloseFilename(sortida);
}

////////////////////////////////////////////////////////////////////////////////

void StructHelperGenerator::GenerateCpp()
{
  string filename = m_className + ".cpp";
  ofstream* sortida = CreateFilename(m_outputPath + filename);
  if (sortida && sortida->is_open())
  {
    cout << "Creating '" << filename << "'" << endl;
    GenerateImplementation(*sortida);
  }
  CloseFilename(sortida);
}

////////////////////////////////////////////////////////////////////////////////

void StructHelperGenerator::GenerateDefinition(ofstream& sortida)
{
  sortida << "#pragma once" << endl;
  sortida << endl;

  sortida << "namespace dxtraceman" << endl;
  sortida << "{" << endl;
  sortida << "  class " << m_className << endl;
  sortida << "  {" << endl;

  GenerateDefinitionMethods(sortida);

  sortida << "  };" << endl;
  sortida << "}" << endl;
}

////////////////////////////////////////////////////////////////////////////////

void StructHelperGenerator::GenerateDefinitionMethods(ofstream& sortida)
{
  sortida << "  public:" << endl;
  sortida << endl;

  for (vector<StructDescriptionPtr>::iterator it = m_structures.begin(); it != m_structures.end(); it++)
  {
    sortida << "    static int " << (*it)->GetName() << "_ToString(char* buffer, unsigned int size, " << (*it)->GetName() << "* value);" << endl;
  }

  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void StructHelperGenerator::GenerateImplementation(ofstream& sortida)
{
  sortida << "#include \"stdafx.h\"" << endl;
  sortida << "#include \"arraystream.h\"" << endl;
  sortida << "#include \"DXTypeHelper.h\"" << endl;
  sortida << "#include \"" << m_className << ".h\"" << endl;
  sortida << endl;
  sortida << "using namespace dxtraceman;" << endl;
  sortida << endl;

  for (vector<StructDescriptionPtr>::iterator it = m_structures.begin(); it != m_structures.end(); it++)
  {
    GenerateImplementationMethod(sortida, *it);
  }
}

////////////////////////////////////////////////////////////////////////////////

void StructHelperGenerator::GenerateImplementationMethod(ofstream& sortida, StructDescriptionPtr structure)
{
  StructSpecificCodePtr structSC = m_config.GetStructSpecificCode(structure->GetName());
  
  // Begin generate method body
  sortida << "int " << m_className << "::" << structure->GetName() << "_ToString(char* buffer, unsigned int size, " << structure->GetName() << "* value)" << endl;
  sortida << "{" << endl;
  
  sortida << "  char local_buffer[256];" << endl;
  sortida << "  arraystream buf(buffer, size);" << endl;
  sortida << "  std::ostream out(&buf);" << endl;
  sortida << endl;

  sortida << "  out << \"# struct " << structure->GetName() << "\" << std::endl;" << endl;
  sortida << "  out << \"# {\" << std::endl;" << endl;
  for (unsigned int i=0; i < structure->GetFieldCount(); i++)
  {
    string fieldType = structure->GetField(i)->GetType();
    string fieldName = structure->GetField(i)->GetName();
    if (structSC)
    {
      StructFieldSpecificCodePtr fieldSC = structSC->GetField(fieldName);
      if ((bool) fieldSC && (bool) fieldSC->GetPolicy(StructFieldSpecificCode::ChangeSaveType))
      {
        StructFieldSpecificCode::StructFieldChangeSaveTypePolicy* fieldPolicy = (StructFieldSpecificCode::StructFieldChangeSaveTypePolicy*) (void*) fieldSC->GetPolicy(StructFieldSpecificCode::ChangeSaveType);
        fieldType = fieldPolicy->GetSaveType();
      }
    }
    sortida << "  DXTypeHelper::ToString(local_buffer, sizeof(local_buffer), (void*) &(value->" << fieldName << "), DXTypeHelper::TT_" << fieldType << ");" << endl;
    sortida << "  out << \"#   " << fieldName << " = \" << local_buffer << std::endl;" << endl;
  }
  sortida << "  out << \"# }\";" << endl;
  sortida << endl;
  sortida << "  buf.finalize();" << endl;
  sortida << "  return buf.tellp();" << endl;

  // End generate method body
  sortida << "}" << endl;
  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////
