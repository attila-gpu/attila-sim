////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXCodeGenException.h"
#include "Utilities/FileSystem.h"
#include "Utilities/String.h"
#include "Utilities/System.h"
#include "Items/CppMacro.h"
#include "Config/ParserConfiguration.h"
#include "Config/GeneratorConfiguration.h"
#include "Config/ConfigManager.h"

using namespace std;
using namespace regex;
using namespace dxcodegen;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;

////////////////////////////////////////////////////////////////////////////////

ConfigManager::ConfigManager(const string& filename, bool verbose, bool createIfNotExists) :
m_filename(filename),
m_verbose(verbose)
{
  bool loadOkay = m_xmlDocument.LoadFile(m_filename);
  if (!loadOkay)
  {
    // ErrorId = 2: File not found
    if (m_xmlDocument.ErrorId()==2 && createIfNotExists)
    {
      FillDefault();
      Save();
    }
    else
    {
      DXCodeGenException e("Could'nt load '" + m_filename + "'. XMLParserError='" + m_xmlDocument.ErrorDesc() + "'");
      throw e;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

ConfigManager::~ConfigManager()
{
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::GetParserConfiguration(ParserConfiguration& config)
{
  config.Clear();

  if (m_verbose)
  {
    cout << "Loading parser configuration from '" << m_filename << "'" << endl;
    cout << "-----------------------------------";
    for (size_t i=0; i < m_filename.length(); i++)
    {
      cout << "-";
    }
    cout << "-" << endl;
  }

  AddParserFiles(config);
  AddParserEnums(config);
  AddParserStructs(config);
  AddParserClasses(config);
  AddParserMacros(config);

  if (m_verbose)
  {
    cout << "-----------------------------------";
    for (size_t i=0; i < m_filename.length(); i++)
    {
      cout << "-";
    }
    cout << "-" << endl;
    cout << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::GetGeneratorConfiguration(GeneratorConfiguration& config)
{
  config.Clear();

  if (m_verbose)
  {
    cout << "Loading generator configuration from '" << m_filename << "'" << endl;
    cout << "--------------------------------------";
    for (size_t i=0; i < m_filename.length(); i++)
    {
      cout << "-";
    }
    cout << "-" << endl;
  }

  AddGeneratorAttributes(config);
  AddGeneratorTypesSpecificCode(config);
  AddGeneratorStructsSpecificCode(config);
  AddGeneratorClassesSpecificCode(config);

  if (m_verbose)
  {
    cout << "--------------------------------------";
    for (size_t i=0; i < m_filename.length(); i++)
    {
      cout << "-";
    }
    cout << "-" << endl;
    cout << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddParserFiles(ParserConfiguration& config)
{
  TiXmlNode* root = GetSection("configuration/parser");
  if (root)
  {
    TiXmlNode* node = NULL;
    while (node = root->IterateChildren("files", node))
    {
      string basepath;
      TiXmlElement* files = node->ToElement();
      if (files->Attribute("basepath"))
      {
        basepath = files->Attribute("basepath");
      }

      TiXmlNode* child = NULL;
      while (child = node->IterateChildren("file", child))
      {
        TiXmlElement* file = child->ToElement();
        if (file->GetText())
        {
          string filename = file->GetText();
          Utilities::String::TrimString(filename);
          if (!filename.empty())
          {
            if (m_verbose)
            {
              cout << "Added file '" << basepath + filename << "' to parse" << endl;
            }
            config.AddFile(basepath + filename);
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddParserEnums(ParserConfiguration& config)
{
  vector<string>* vec_str;
  vector<string>::iterator it;

  vec_str = GetTextList("configuration/parser/enums", "include");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added enum '" << *it << "' to include list" << endl;
    }
    config.AddEnum(*it, false);
  }
  delete vec_str;

  vec_str = GetTextList("configuration/parser/enums", "exclude");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added enum '" << *it << "' to exclude list" << endl;
    }
    config.AddEnum(*it, true);
  }
  delete vec_str;
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddParserStructs(ParserConfiguration& config)
{
  vector<string>* vec_str;
  vector<string>::iterator it;

  vec_str = GetTextList("configuration/parser/structs", "include");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added struct '" << *it << "' to include list" << endl;
    }
    config.AddStruct(*it, false);
  }
  delete vec_str;

  vec_str = GetTextList("configuration/parser/structs", "exclude");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added struct '" << *it << "' to exclude list" << endl;
    }
    config.AddStruct(*it, true);
  }
  delete vec_str;
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddParserClasses(ParserConfiguration& config)
{
  vector<string>* vec_str;
  vector<string>::iterator it;

  vec_str = GetTextList("configuration/parser/classes", "include");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added class '" << *it << "' to include list" << endl;
    }
    config.AddClass(*it, false);
  }
  delete vec_str;

  vec_str = GetTextList("configuration/parser/classes", "exclude");
  for (it=vec_str->begin(); it != vec_str->end(); it++)
  {
    if (m_verbose)
    {
      cout << "Added class '" << *it << "' to exclude list" << endl;
    }
    config.AddClass(*it, true);
  }
  delete vec_str;
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddParserMacros(ParserConfiguration& config)
{
  TiXmlNode* root = GetSection("configuration/parser/macros");
  if (root)
  {
    TiXmlNode* node = NULL;
    while (node = root->IterateChildren("macro", node))
    {
      const char* p;
      string left;
      p = node->FirstChild("left")->ToElement()->GetText();
      if (p) left = p;
      string right;
      p = node->FirstChild("right")->ToElement()->GetText();
      if (p) right = p;
      if (!left.empty())
      {
        CppMacroPtr macro = new CppMacro(left, right);
        if (m_verbose)
        {
          cout << "Added macro '" << macro->GetLeft() << "'" << endl;
        }
        config.AddMacro(macro);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddGeneratorAttributes(GeneratorConfiguration& config)
{
  TiXmlNode* root;

  root = GetSection("configuration/generator");
  if (root)
  {
    TiXmlElement* generator = root->ToElement();
    if (generator->Attribute("outputpath"))
    {
      config.SetOutputPath(generator->Attribute("outputpath"));
      if (m_verbose)
      {
        cout << "Output path '" << config.GetOutputPath() << "'" << endl;
      }
    }
  }

  root = GetSection("configuration/generator/classes");
  if (root)
  {
    TiXmlElement* classes = root->ToElement();

    if (classes->Attribute("wrappersuffix"))
    {
      config.SetWrapperSuffix(classes->Attribute("wrappersuffix"));
    }

    if (classes->Attribute("stubsuffix"))
    {
      config.SetStubSuffix(classes->Attribute("stubsuffix"));
    }

    if (classes->Attribute("wrapperbaseclass"))
    {
      config.SetWrapperBaseClass(classes->Attribute("wrapperbaseclass"));
    }

    if (classes->Attribute("stubbaseclass"))
    {
      config.SetStubBaseClass(classes->Attribute("stubbaseclass"));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddGeneratorTypesSpecificCode(GeneratorConfiguration& config)
{
  TiXmlNode* root = GetSection("configuration/generator/types");
  if (root)
  {
    TiXmlNode* typeNode = NULL;
    while (typeNode = root->IterateChildren("type", typeNode))
    {
      if (typeNode->ToElement()->Attribute("name") &&
          typeNode->ToElement()->Attribute("savein") &&
          typeNode->ToElement()->Attribute("usemethod") &&
          typeNode->ToElement()->Attribute("passby") &&
          !config.GetTypeSpecificCode(typeNode->ToElement()->Attribute("name")))
      {
        string typeName = typeNode->ToElement()->Attribute("name");
        TypeSpecificCode::SaveIn saveIn;
        string useMethod = typeNode->ToElement()->Attribute("usemethod");
        TypeSpecificCode::PassBy passBy;
        
        string saveInName = typeNode->ToElement()->Attribute("savein");
        if (saveInName.compare("callstack") == 0)
          saveIn = TypeSpecificCode::SI_CallStack;
        else if (saveInName.compare("buffer") == 0)
          saveIn = TypeSpecificCode::SI_Buffer;
        else
          continue;
        
        string passByName = typeNode->ToElement()->Attribute("passby");
        if (passByName.compare("value") == 0)
          passBy = TypeSpecificCode::PB_Value;
        else if (passByName.compare("reference") == 0)
          passBy = TypeSpecificCode::PB_Reference;
        else
          continue;
        
        TypeSpecificCodePtr typeSC = new TypeSpecificCode(typeName, saveIn, useMethod, passBy);
        config.AddTypeSpecificCode(typeSC);

        if (m_verbose)
        {
          cout << "Added specific code type '" << typeName << "'" << endl;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddGeneratorStructsSpecificCode(GeneratorConfiguration& config)
{
  TiXmlNode* root = GetSection("configuration/generator/structs");
  if (root)
  {
    TiXmlNode* structNode = NULL;
    while (structNode = root->IterateChildren("struct", structNode))
    {
      if (structNode->ToElement()->Attribute("name"))
      {
        string structName = structNode->ToElement()->Attribute("name");
        
        if (!config.GetStructSpecificCode(structName))
        {
          StructSpecificCodePtr structSC = new StructSpecificCode(structName);

          TiXmlNode* fieldNode = NULL;
          while (fieldNode = structNode->IterateChildren("field", fieldNode))
          {
            if (fieldNode->ToElement()->Attribute("name") && fieldNode->ToElement()->Attribute("policy"))
            {
              string fieldName = fieldNode->ToElement()->Attribute("name");

              if (!structSC->GetField(fieldName))
              {
                StructFieldSpecificCodePtr fieldSC = new StructFieldSpecificCode(fieldName);

                string policyName = fieldNode->ToElement()->Attribute("policy");

                StructFieldSpecificCode::StructFieldPolicyPtr policyPointer = NULL;

                if (policyName.compare("changesavetype") == 0)
                {
                  string saveType = fieldNode->ToElement()->GetText();
                  if (!saveType.empty())
                  {
                    policyPointer = new StructFieldSpecificCode::StructFieldChangeSaveTypePolicy(saveType);
                  }
                }

                if (policyPointer)
                {
                  fieldSC->AddPolicy(policyPointer);
                  structSC->AddField(fieldSC);
                }
              }
            }
          }

          if (structSC->GetFieldCount() > 0)
          {
            config.AddStructSpecificCode(structSC);

            if (m_verbose)
            {
              cout << "Added specific code struct '" << structName << "'" << endl;

              vector<string>* v_names = structSC->GetFieldNames();
              for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
              {
                cout << "      field name " << *it << ": policies = ";
                for (unsigned int i=0; i < structSC->GetField(*it)->GetPolicyCount(); i++)
                {
                  if (i > 0) cout << ", ";
                  cout << structSC->GetField(*it)->GetPolicy(i)->GetName();
                }
                cout << endl;
              }
              delete v_names;
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddGeneratorClassesSpecificCode(GeneratorConfiguration& config)
{
  TiXmlNode* root = GetSection("configuration/generator/classes");
  if (root)
  {
    TiXmlNode* classNode = NULL;
    while (classNode = root->IterateChildren("class", classNode))
    {
      if (classNode->ToElement()->Attribute("name"))
      {
        string className = classNode->ToElement()->Attribute("name");

        if (!config.GetClassSpecificCode(className))
        {
          ClassSpecificCodePtr classSC = new ClassSpecificCode(className);

          AddGeneratorClassesSpecificCodeWrapperAttribsAdd(classNode->ToElement()->FirstChild("attribs_add"), classSC);
          AddGeneratorClassesSpecificCodeStubAttribsAdd(classNode->ToElement()->FirstChild("attribs_add"), classSC);
          AddGeneratorClassesSpecificCodeWrapperMethodsAdd(classNode->ToElement()->FirstChild("methods_add"), classSC);
          AddGeneratorClassesSpecificCodeStubMethodsAdd(classNode->ToElement()->FirstChild("methods_add"), classSC);
          AddGeneratorClassesSpecificCodeMethods(classNode->ToElement()->FirstChild("methods"), classSC);

          if (classSC->GetWrapperAttributeCount() > 0 || classSC->GetStubAttributeCount() > 0 || classSC->GetWrapperNewMethodCount() > 0 || classSC->GetStubNewMethodCount() > 0 || classSC->GetMethodCount() > 0)
          {
            config.AddClassSpecificCode(classSC);

            if (m_verbose)
            {
              vector<string>* v_names;
              
              v_names = classSC->GetWrapperAttributeNames();
              for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
              {
                cout << "Added specific code wrapper attrib " << className << "::" << *it << endl;
              }
              delete v_names;

              v_names = classSC->GetStubAttributeNames();
              for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
              {
                cout << "Added specific code stub attrib " << className << "::" << *it << endl;
              }
              delete v_names;

              v_names = classSC->GetWrapperNewMethodNames();
              for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
              {
                cout << "Added specific code new wrapper method " << className << "::" << *it << endl;
              }
              delete v_names;

              v_names = classSC->GetStubNewMethodNames();
              for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
              {
                cout << "Added specific code new stub method " << className << "::" << *it << endl;
              }
              delete v_names;
              
              v_names = classSC->GetMethodNames();
              for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
              {
                cout << "Added specific code method " << className << "::" << *it << "," << endl;
                
                MethodSpecificCodePtr methodSC = classSC->GetMethod(*it);
                
                vector<unsigned int>* v_positions = methodSC->GetParamPositions();
                for (vector<unsigned int>::iterator it = v_positions->begin(); it != v_positions->end(); it++)
                {
                  cout << "      param position " << (unsigned int) *it << ": policies = ";
                  for (unsigned int i=0; i < methodSC->GetParam((unsigned int) *it)->GetPolicyCount(); i++)
                  {
                    if (i > 0) cout << ", ";
                    cout << methodSC->GetParam((unsigned int) *it)->GetPolicy(i)->GetName();
                  }
                  cout << endl;
                }
                delete v_positions;

                if (methodSC->GetPolicy())
                {
                  cout << "      method: policy = " << methodSC->GetPolicy()->GetName() << endl;
                }
              }
              delete v_names;
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddGeneratorClassesSpecificCodeWrapperAttribsAdd(TiXmlNode* classNode, ClassSpecificCodePtr classSC)
{
  if (!classNode || !(classNode = classNode->ToElement()->FirstChild("wrapper")))
  {
    return;
  }
  
  TiXmlNode* attribNode = NULL;
  while (attribNode = classNode->IterateChildren("attrib", attribNode))
  {
    if (attribNode->ToElement()->Attribute("name"))
    {
      string attribName = attribNode->ToElement()->Attribute("name");
      
      if (!classSC->GetWrapperAttribute(attribName))
      {
        AttributeSpecificCodePtr attribSC = new AttributeSpecificCode(attribName, attribNode->ToElement()->GetText());
        classSC->AddWrapperAttribute(attribSC);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddGeneratorClassesSpecificCodeStubAttribsAdd(TiXmlNode* classNode, ClassSpecificCodePtr classSC)
{
  if (!classNode || !(classNode = classNode->ToElement()->FirstChild("stub")))
  {
    return;
  }

  TiXmlNode* attribNode = NULL;
  while (attribNode = classNode->IterateChildren("attrib", attribNode))
  {
    if (attribNode->ToElement()->Attribute("name"))
    {
      string attribName = attribNode->ToElement()->Attribute("name");

      if (!classSC->GetStubAttribute(attribName))
      {
        AttributeSpecificCodePtr attribSC = new AttributeSpecificCode(attribName, attribNode->ToElement()->GetText());
        classSC->AddStubAttribute(attribSC);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddGeneratorClassesSpecificCodeWrapperMethodsAdd(TiXmlNode* classNode, Items::ClassSpecificCodePtr classSC)
{
  if (!classNode || !(classNode = classNode->ToElement()->FirstChild("wrapper")))
  {
    return;
  }

  TiXmlNode* methodNode = NULL;
  while (methodNode = classNode->IterateChildren("method", methodNode))
  {
    if (methodNode->ToElement()->Attribute("name") && methodNode->ToElement()->Attribute("public"))
    {
      string methodName = methodNode->ToElement()->Attribute("name");

      if (!classSC->GetWrapperNewMethod(methodName))
      {
        NewMethodSpecificCodePtr newMethodSC = new NewMethodSpecificCode(methodName);

        bool isPublic = false;
        if (methodNode->ToElement()->Attribute("public"))
        {
          string publicValue = methodNode->ToElement()->Attribute("public");
          isPublic = (publicValue.compare("true") == 0 || publicValue.compare("1") == 0);
        }

        newMethodSC->SetIsPublic(isPublic);
        newMethodSC->SetDefinition(methodNode->ToElement()->GetText());

        classSC->AddWrapperNewMethod(newMethodSC);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddGeneratorClassesSpecificCodeStubMethodsAdd(TiXmlNode* classNode, Items::ClassSpecificCodePtr classSC)
{
  if (!classNode || !(classNode = classNode->ToElement()->FirstChild("stub")))
  {
    return;
  }

  TiXmlNode* methodNode = NULL;
  while (methodNode = classNode->IterateChildren("method", methodNode))
  {
    if (methodNode->ToElement()->Attribute("name") && methodNode->ToElement()->Attribute("public"))
    {
      string methodName = methodNode->ToElement()->Attribute("name");

      if (!classSC->GetStubNewMethod(methodName))
      {
        NewMethodSpecificCodePtr newMethodSC = new NewMethodSpecificCode(methodName);

        bool isPublic = false;
        if (methodNode->ToElement()->Attribute("public"))
        {
          string publicValue = methodNode->ToElement()->Attribute("public");
          isPublic = (publicValue.compare("true") == 0 || publicValue.compare("1") == 0);
        }

        newMethodSC->SetIsPublic(isPublic);
        newMethodSC->SetDefinition(methodNode->ToElement()->GetText());

        classSC->AddStubNewMethod(newMethodSC);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::AddGeneratorClassesSpecificCodeMethods(TiXmlNode* classNode, Items::ClassSpecificCodePtr classSC)
{
  if (!classNode)
  {
    return;
  }
  
  TiXmlNode* methodNode = NULL;
  while (methodNode = classNode->IterateChildren("method", methodNode))
  {
    if (methodNode->ToElement()->Attribute("name"))
    {
      string methodName = methodNode->ToElement()->Attribute("name");

      if (!classSC->GetMethod(methodName))
      {
        MethodSpecificCodePtr methodSC = new MethodSpecificCode(methodName);
        
        TiXmlNode* paramNode = NULL;
        while (paramNode = methodNode->IterateChildren("param", paramNode))
        {
          if (paramNode->ToElement()->Attribute("position") && paramNode->ToElement()->Attribute("policy"))
          {
            unsigned int paramPosition = atoi(paramNode->ToElement()->Attribute("position"));
            
            if (paramPosition > 0 && !methodSC->GetParam(paramPosition))
            {
              ParamSpecificCodePtr paramSC = new ParamSpecificCode(paramPosition);

              string policiesNames = paramNode->ToElement()->Attribute("policy");
              
              static const rpattern patro_divisio("\\s* , \\s*", EXTENDED);
              split_results results;
              if (patro_divisio.split(policiesNames, results) > 0)
              {
                for (split_results::iterator it=results.begin(); it != results.end(); it++)
                {
                  ParamSpecificCode::ParamPolicyPtr policyPointer = NULL;

                  string policyName = *it;
                  if (policyName.compare("nocapture") == 0)
                  {
                    policyPointer = new ParamSpecificCode::NoCapturePolicy();
                  }
                  else if (policyName.compare("changesavetype") == 0)
                  {
                    string saveType = paramNode->ToElement()->GetText();
                    if (!saveType.empty())
                    {
                      policyPointer = new ParamSpecificCode::ChangeSaveTypePolicy(saveType);
                    }
                  }
                  else if (policyName.compare("savespecific") == 0)
                  {
                    policyPointer = new ParamSpecificCode::SaveSpecificPolicy();
                  }
                  else if (policyName.compare("helpparams") == 0)
                  {
                    if (paramNode->ToElement()->Attribute("params"))
                    {
                      policyPointer = new ParamSpecificCode::HelpParamsPolicy(paramNode->ToElement()->Attribute("params"));
                    }
                  }

                  if (policyPointer)
                  {
                    paramSC->AddPolicy(policyPointer);
                    methodSC->AddParam(paramSC);
                  }
                }
              }
            }
          }
        }

        TiXmlNode* specificNode = methodNode->FirstChild("specific");
        if (specificNode && specificNode->ToElement()->Attribute("policy"))
        {
          string policyName = specificNode->ToElement()->Attribute("policy");
          
          MethodSpecificCode::MethodPolicyPtr policyPointer = NULL;
          if (policyName.compare("replaceboth") == 0)
          {
            bool passCallSaver = false;
            if (specificNode->ToElement()->Attribute("passcallsaver"))
            {
              string passCallSaverValue = specificNode->ToElement()->Attribute("passcallsaver");
              passCallSaver = (passCallSaverValue.compare("true") == 0 || passCallSaverValue.compare("1") == 0);
            }

            policyPointer = new MethodSpecificCode::ReplaceBothPolicy(specificNode->ToElement()->GetText(), passCallSaver);
          }
          else if (policyName.compare("replacewrapperonly") == 0)
          {
            bool passCallSaver = false;
            if (specificNode->ToElement()->Attribute("passcallsaver"))
            {
              string passCallSaverValue = specificNode->ToElement()->Attribute("passcallsaver");
              passCallSaver = (passCallSaverValue.compare("true") == 0 || passCallSaverValue.compare("1") == 0);
            }

            policyPointer = new MethodSpecificCode::ReplaceWrapperOnlyPolicy(specificNode->ToElement()->GetText(), passCallSaver);
          }
          else if (policyName.compare("replacestubonly") == 0)
          {
            policyPointer = new MethodSpecificCode::ReplaceStubOnlyPolicy();
          }

          if (policyPointer)
          {
            methodSC->SetPolicy(policyPointer);
          }
        }
        
        if (methodSC->GetParamCount() > 0 || (bool) methodSC->GetPolicy())
        {
          classSC->AddMethod(methodSC);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::AddHeader()
{
  if (!ExistsHeader())
  {
    m_xmlDocument.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", "yes"));
  }
  if (!m_xmlDocument.RootElement())
  {
    TiXmlElement* section = new TiXmlElement("configuration");
    m_xmlDocument.LinkEndChild(section);
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::AddSection(string sectionParent, string sectionName)
{
  TiXmlNode* root = GetSection(sectionParent);
  if (root)
  {
    TiXmlElement* childSection = new TiXmlElement(sectionName);
    root->LinkEndChild(childSection);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::AddSection(string sectionParent, string sectionName, string sectionText)
{
  TiXmlNode* root = GetSection(sectionParent);
  if (root)
  {
    TiXmlElement* childSection = new TiXmlElement(sectionName);
    childSection->LinkEndChild(new TiXmlText(sectionText));
    root->LinkEndChild(childSection);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::AddSectionAttribute(string sectionName, string attribName, string attribValue)
{
  TiXmlNode* root = GetSection(sectionName);
  if (root)
  {
    TiXmlElement* section = root->ToElement();
    section->SetAttribute(attribName, attribValue);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::ExistsHeader()
{
  if (m_xmlDocument.NoChildren())
  {
    return false;
  }
  else
  {
    TiXmlNode* node = m_xmlDocument.FirstChild();
    if (node->Type() != TiXmlNode::DECLARATION)
    {
      return false;
    }
    else
    {
      TiXmlDeclaration* declaration = node->ToDeclaration();

      string cadena = declaration->Version();
      if (cadena != "1.0")
      {
        node->Parent()->RemoveChild(node);
        return false;
      }

      cadena = declaration->Encoding();
      if (cadena != "utf-8")
      {
        node->Parent()->RemoveChild(node);
        return false;
      }

      cadena = declaration->Standalone();
      if (cadena != "yes")
      {
        node->Parent()->RemoveChild(node);
        return false;
      }

      return true;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

TiXmlNode* ConfigManager::GetSection(string sectionName)
{
  TiXmlNode* node = NULL;
  static const rpattern patro_divisio("/");
  split_results results;
  if (patro_divisio.split(sectionName, results) > 0)
  {
    bool problem = m_xmlDocument.NoChildren();
    split_results::iterator it;

    for (it=results.begin(); it != results.end() && !problem; it++)
    {
      if (it == results.begin())
        node = m_xmlDocument.RootElement();
      else
        node = node->FirstChild(*it);

      if (node)
        problem = (node->ToElement()->ValueStr() != *it);
      else
        problem = true;
    }

    if (problem)
      node = NULL;
  }

  return node;
}

////////////////////////////////////////////////////////////////////////////////

vector<string>* ConfigManager::GetTextList(string sectionName, string subSectionName)
{
  vector<string>* llista = new vector<string>();

  TiXmlNode* root = GetSection(sectionName);
  if (root)
  {
    TiXmlNode* node = NULL;
    while (node = root->IterateChildren(subSectionName, node))
    {
      TiXmlElement* elem = node->ToElement();
      if (elem->GetText())
      {
        string cadena = elem->GetText();
        Utilities::String::TrimString(cadena);
        if (!cadena.empty())
        {
          llista->push_back(cadena);
        }
      }
    }
  }

  return llista;
}

////////////////////////////////////////////////////////////////////////////////

void ConfigManager::Clear()
{
  m_xmlDocument.Clear();
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::FillDefault()
{
  Clear();

  AddHeader();

  AddSection("configuration", "parser");
  AddSection("configuration/parser", "files");
  AddSectionAttribute("configuration/parser/files", "basepath", GetDirectXSDKPath());

  AddSection("configuration/parser/files", "file", "d3d9.h");
  AddSection("configuration/parser/files", "file", "d3d9caps.h");
  AddSection("configuration/parser/files", "file", "d3d9types.h");

  AddSection("configuration/parser", "enums");
  AddSection("configuration/parser/enums", "include");
  AddSection("configuration/parser/enums", "exclude");

  AddSection("configuration/parser", "structs");
  AddSection("configuration/parser/structs", "include");
  AddSection("configuration/parser/structs", "exclude");

  AddSection("configuration/parser", "classes");
  AddSection("configuration/parser/classes", "include");
  AddSection("configuration/parser/classes", "exclude");

  AddSection("configuration/parser", "macros");
  AddSection("configuration/parser/macros", "macro");
  AddSection("configuration/parser/macros/macro", "left", "");
  AddSection("configuration/parser/macros/macro", "right", "");

  AddSection("configuration", "generator");
  AddSectionAttribute("configuration/generator", "outputpath", "");

  AddSection("configuration/generator", "classes");
  AddSectionAttribute("configuration/generator/classes", "namesuffix", "Interceptor");
  AddSectionAttribute("configuration/generator/classes", "baseclass", "DXInterceptor");

  return false;
}

////////////////////////////////////////////////////////////////////////////////

string ConfigManager::GetDirectXSDKPath()
{
  string pathSDK;
  Utilities::System::ReadEnvironmentVariable("DXSDK_DIR", pathSDK);
  if (!pathSDK.empty() && Utilities::FileSystem::DirectoryExists(pathSDK))
  {
    Utilities::FileSystem::DirectoryAddBackslash(pathSDK);
    pathSDK += "include\\";
    if (!Utilities::FileSystem::DirectoryExists(pathSDK))
    {
      pathSDK.clear();
    }
  }
  else
  {
    pathSDK.clear();
  }
  return pathSDK;
}

////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::Save()
{
  if (!m_filename.empty())
  {
    m_xmlDocument.SaveFile(m_filename);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
