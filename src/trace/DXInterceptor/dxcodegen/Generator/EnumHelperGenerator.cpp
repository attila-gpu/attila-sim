////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Generator/EnumHelperGenerator.h"

using namespace std;
using namespace dxcodegen::Items;
using namespace dxcodegen::Generator;

////////////////////////////////////////////////////////////////////////////////

EnumHelperGenerator::EnumHelperGenerator(const std::vector<EnumDescriptionPtr>& enumerations, const string& outputPath) :
m_outputPath(outputPath),
m_className("DXEnumHelper"),
IGenerator("class DXEnumHelper")
{
  m_enumerations = enumerations;
}

////////////////////////////////////////////////////////////////////////////////

EnumHelperGenerator::~EnumHelperGenerator()
{
}

////////////////////////////////////////////////////////////////////////////////

void EnumHelperGenerator::GenerateCode()
{
  GenerateHpp();
  GenerateCpp();
}

////////////////////////////////////////////////////////////////////////////////

void EnumHelperGenerator::GenerateHpp()
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

void EnumHelperGenerator::GenerateCpp()
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

void EnumHelperGenerator::GenerateDefinition(ofstream& sortida)
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

void EnumHelperGenerator::GenerateDefinitionMethods(ofstream& sortida)
{
  sortida << "  public:" << endl;
  sortida << endl;

  for (vector<EnumDescriptionPtr>::iterator it = m_enumerations.begin(); it != m_enumerations.end(); it++)
  {
    string enumName = (*it)->GetName();
    
    sortida << "    // enum " << enumName << endl;
    sortida << "    static std::map<" << enumName << ", char*> ms_dict_" << enumName << ";" << endl;
    sortida << "    static void " << enumName << "_InitializeDictionary();" << endl;
    sortida << "    static int " << enumName << "_ToString(char* buffer, unsigned int size, " << enumName << " value);" << endl;
    sortida << "    static const char* " << enumName << "_ToStringValue(" << enumName << " value);" << endl;
    sortida << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

void EnumHelperGenerator::GenerateImplementation(ofstream& sortida)
{
  sortida << "#include \"stdafx.h\"" << endl;
  sortida << "#include \"" << m_className << ".h\"" << endl;
  sortida << endl;
  
  sortida << "using namespace dxtraceman;" << endl;
  sortida << endl;
  
  sortida << "#ifdef _MSC_VER" << endl;
  sortida << "#define snprintf _snprintf" << endl;
  sortida << "#endif" << endl;
  sortida << endl;

  for (vector<EnumDescriptionPtr>::iterator it = m_enumerations.begin(); it != m_enumerations.end(); it++)
  {
    GenerateImplementationDictInstance(sortida, *it);
    GenerateImplementationDictInitialization(sortida, *it);
    GenerateImplementationDictMethods(sortida, *it);
  }
}

////////////////////////////////////////////////////////////////////////////////

void EnumHelperGenerator::GenerateImplementationDictInstance(ofstream& sortida, EnumDescriptionPtr enumeration)
{
  string enumName = enumeration->GetName();
  
  sortida << "std::map<" << enumName << ", char*> " << m_className << "::ms_dict_" << enumName << ";" << endl;
  sortida << "namespace { bool " << enumName << "_initialiser = (" << m_className << "::" << enumName << "_InitializeDictionary(), true); } // simulate a static constructor in C++" << endl;
  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void EnumHelperGenerator::GenerateImplementationDictInitialization(ofstream& sortida, EnumDescriptionPtr enumeration)
{
  // Begin generate method body
  sortida << "void " << m_className << "::" << enumeration->GetName() << "_InitializeDictionary()" << endl;
  sortida << "{" << endl;

  // Body generate hash table initialization [
  for (unsigned int i=0, j=enumeration->GetMembersCount(); i < j; i++)
  {
    sortida << "  " << m_className << "::ms_dict_" << enumeration->GetName() << ".insert(std::pair<" << enumeration->GetName() << ", char*>(" << enumeration->GetMember(i)->GetName() << ", \"" << enumeration->GetMember(i)->GetName() << "\"));" << endl;
  }
  // ]

  // End generate method body
  sortida << "}" << endl;
  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void EnumHelperGenerator::GenerateImplementationDictMethods(ofstream& sortida, EnumDescriptionPtr enumeration)
{
  string enumName = enumeration->GetName();

  // Begin generate method body
  sortida << "int " << m_className << "::" << enumName << "_ToString(char* buffer, unsigned int size, " << enumName << " value)" << endl;
  sortida << "{" << endl;
  
  sortida << "  std::map<" << enumName << ", char*>::iterator enumName = " << m_className << "::ms_dict_" << enumName << ".find(value);" << endl;
  sortida << "  if (enumName != ms_dict_" << enumName << ".end())" << endl;
  sortida << "  {" << endl;
  sortida << "    return snprintf(buffer, size, \"%s\", (*enumName).second);" << endl;
  sortida << "  }" << endl;
  sortida << "  else" << endl;
  sortida << "  {" << endl;
  sortida << "    return snprintf(buffer, size, \"ERROR_ENUM_CONSTANT_NOT_FOUND\");" << endl;
  sortida << "  }" << endl;
  
  // End generate method body
  sortida << "}" << endl;
  sortida << endl;

  // Begin generate method body
  sortida << "const char* " << m_className << "::" << enumName << "_ToStringValue(" << enumName << " value)" << endl;
  sortida << "{" << endl;

  sortida << "  std::map<" << enumName << ", char*>::iterator enumName = " << m_className << "::ms_dict_" << enumName << ".find(value);" << endl;
  sortida << "  if (enumName != ms_dict_" << enumName << ".end())" << endl;
  sortida << "  {" << endl;
  sortida << "    return (*enumName).second;" << endl;
  sortida << "  }" << endl;
  sortida << "  else" << endl;
  sortida << "  {" << endl;
  sortida << "    return \"ERROR_ENUM_CONSTANT_NOT_FOUND\";" << endl;
  sortida << "  }" << endl;

  // End generate method body
  sortida << "}" << endl;
  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////
