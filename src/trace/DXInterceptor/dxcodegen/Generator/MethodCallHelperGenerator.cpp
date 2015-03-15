////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Generator/MethodCallHelperGenerator.h"

using namespace std;
using namespace dxcodegen::Items;
using namespace dxcodegen::Generator;

////////////////////////////////////////////////////////////////////////////////

MethodCallHelperGenerator::MethodCallHelperGenerator(const vector<ClassDescriptionPtr>& classes, const string& outputPath) :
m_outputPath(outputPath),
m_className("DXMethodCallHelper"),
IGenerator("class DXMethodCallHelper")
{
  m_classes = classes;
}

////////////////////////////////////////////////////////////////////////////////

MethodCallHelperGenerator::~MethodCallHelperGenerator()
{
}

////////////////////////////////////////////////////////////////////////////////

void MethodCallHelperGenerator::GenerateCode()
{
  GenerateHpp();
  GenerateCpp();
}

////////////////////////////////////////////////////////////////////////////////

void MethodCallHelperGenerator::GenerateHpp()
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

void MethodCallHelperGenerator::GenerateCpp()
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

void MethodCallHelperGenerator::GenerateDefinition(ofstream& sortida)
{
  sortida << "#pragma once" << endl;
  sortida << endl;
  
  sortida << "namespace dxtraceman" << endl;
  sortida << "{" << endl;
  sortida << "  class " << m_className << endl;
  sortida << "  {" << endl;

  GenerateMethodCallTokens(sortida);
  GenerateHelperMethodsHeaders(sortida);

  sortida << "  };" << endl;
  sortida << "}" << endl;
}

////////////////////////////////////////////////////////////////////////////////

void MethodCallHelperGenerator::GenerateMethodCallTokens(ofstream& sortida)
{
  sortida << "  public:" << endl;
  sortida << endl;

  sortida << "    enum DXMethodCallToken" << endl;
  sortida << "    {" << endl;
  sortida << "      TOK_DummyCall = 1," << endl;
  sortida << "      TOK_Direct3DCreate9 = 2," << endl;
  
  unsigned int k = 11;
  for (vector<ClassDescriptionPtr>::iterator it = m_classes.begin(); it != m_classes.end(); it++)
  {
    for (unsigned int i=0, j=(*it)->GetMethodsCount(); i < j; i++, k++)
    {
      sortida << "      TOK_" << (*it)->GetName() << "_" << (*it)->GetMethod(i)->GetName() << " = " << k << "," << endl;
    }
  }

  sortida << "    };" << endl;

  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void MethodCallHelperGenerator::GenerateHelperMethodsHeaders(std::ofstream& sortida)
{
  sortida << "    static std::map<DXMethodCallToken, char*> ms_classNames;" << endl;
  sortida << "    static std::map<DXMethodCallToken, char*> ms_methodNames;" << endl;
  sortida << endl;
  
  sortida << "    static void GetClassName_InitializeDictionary();" << endl;
  sortida << "    static void GetMethodName_InitializeDictionary();" << endl;
  sortida << endl;
  
  sortida << "    static const char* GetClassName(DXMethodCallToken token);" << endl;
  sortida << "    static const char* GetMethodName(DXMethodCallToken token);" << endl;
  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void MethodCallHelperGenerator::GenerateImplementation(ofstream& sortida)
{
  sortida << "#include \"stdafx.h\"" << endl;
  sortida << "#include \"" << m_className << ".h\"" << endl;
  sortida << endl;
  sortida << "using namespace dxtraceman;" << endl;
  sortida << endl;

  GenerateImplementationGetClassName(sortida);
  GenerateImplementationGetMethodName(sortida);
}

////////////////////////////////////////////////////////////////////////////////

void MethodCallHelperGenerator::GenerateImplementationGetClassName(ofstream& sortida)
{
  // ClassName dictionary instance [
  sortida << "std::map<" << m_className << "::DXMethodCallToken, char*> " << m_className << "::ms_classNames;" << endl;
  sortida << "namespace { bool GetClassName_initialiser = (" << m_className << "::GetClassName_InitializeDictionary(), true); } // simulate a static constructor in C++" << endl;
  sortida << endl;
  // ]

  // ClassName dictionary initialization [
  sortida << "void " << m_className << "::GetClassName_InitializeDictionary()" << endl;
  sortida << "{" << endl;

  sortida << "  " << m_className << "::ms_classNames.insert(std::pair<DXMethodCallToken, char*>(TOK_Direct3DCreate9, \"DLL\"));" << endl;
  for (vector<ClassDescriptionPtr>::iterator it = m_classes.begin(); it != m_classes.end(); it++)
  {
    for (unsigned int i=0, j=(*it)->GetMethodsCount(); i < j; i++)
    {
      sortida << "  " << m_className << "::ms_classNames.insert(std::pair<DXMethodCallToken, char*>(TOK_" << (*it)->GetName() << "_" << (*it)->GetMethod(i)->GetName() << ", \"" << (*it)->GetName() << "\"));" << endl;
    }
  }

  sortida << "}" << endl;
  sortida << endl;
  // ]
  
  // ClassName body [
  sortida << "const char* " << m_className << "::GetClassName(DXMethodCallToken token)" << endl;
  sortida << "{" << endl;

  sortida << "  std::map<DXMethodCallToken, char*>::iterator className = " << m_className << "::ms_classNames.find(token);" << endl;
  sortida << "  if (className != ms_classNames.end())" << endl;
  sortida << "  {" << endl;
  sortida << "    return (*className).second;" << endl;
  sortida << "  }" << endl;
  sortida << "  else" << endl;
  sortida << "  {" << endl;
  sortida << "    return \"ERROR_CLASS_NAME_NOT_FOUND\";" << endl;
  sortida << "  }" << endl;
  
  sortida << "}" << endl;
  sortida << endl;
  // ]
}

////////////////////////////////////////////////////////////////////////////////

void MethodCallHelperGenerator::GenerateImplementationGetMethodName(ofstream& sortida)
{
  // MethodName dictionary instance [
  sortida << "std::map<" << m_className << "::DXMethodCallToken, char*> " << m_className << "::ms_methodNames;" << endl;
  sortida << "namespace { bool GetMethodName_initialiser = (" << m_className << "::GetMethodName_InitializeDictionary(), true); } // simulate a static constructor in C++" << endl;
  sortida << endl;
  // ]
  
  // MethodName dictionary initialization [
  sortida << "void " << m_className << "::GetMethodName_InitializeDictionary()" << endl;
  sortida << "{" << endl;

  sortida << "  " << m_className << "::ms_methodNames.insert(std::pair<DXMethodCallToken, char*>(TOK_Direct3DCreate9, \"Direct3DCreate9\"));" << endl;
  for (vector<ClassDescriptionPtr>::iterator it = m_classes.begin(); it != m_classes.end(); it++)
  {
    for (unsigned int i=0, j=(*it)->GetMethodsCount(); i < j; i++)
    {
      sortida << "  " << m_className << "::ms_methodNames.insert(std::pair<DXMethodCallToken, char*>(TOK_" << (*it)->GetName() << "_" << (*it)->GetMethod(i)->GetName() << ", \"" << (*it)->GetMethod(i)->GetName() << "\"));" << endl;
    }
  }

  sortida << "}" << endl;
  sortida << endl;
  // ]
  
  // MethodName body [
  sortida << "const char* " << m_className << "::GetMethodName(DXMethodCallToken token)" << endl;
  sortida << "{" << endl;

  sortida << "  std::map<DXMethodCallToken, char*>::iterator methodName = " << m_className << "::ms_methodNames.find(token);" << endl;
  sortida << "  if (methodName != ms_methodNames.end())" << endl;
  sortida << "  {" << endl;
  sortida << "    return (*methodName).second;" << endl;
  sortida << "  }" << endl;
  sortida << "  else" << endl;
  sortida << "  {" << endl;
  sortida << "    return \"ERROR_METHOD_NAME_NOT_FOUND\";" << endl;
  sortida << "  }" << endl;

  sortida << "}" << endl;
  sortida << endl;
  // ]
}

////////////////////////////////////////////////////////////////////////////////
