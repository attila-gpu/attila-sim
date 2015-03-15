////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Config/GeneratorConfiguration.h"
#include "Items/ClassSpecificCode.h"
#include "Items/ClassDescription.h"
#include "Generator/ClassStubGenerator.h"

using namespace std;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;
using namespace dxcodegen::Generator;

////////////////////////////////////////////////////////////////////////////////

ClassStubGenerator::ClassStubGenerator(GeneratorConfiguration& config, ClassDescriptionPtr cdes, const string& outputPath, const string& classNameSuffix, const string& baseClassName) :
m_config(config),
m_classDescription(cdes),
m_outputPath(outputPath),
IGenerator("class " + cdes->GetName() + classNameSuffix)
{
  m_classSpecificCode = m_config.GetClassSpecificCode(cdes->GetName());
  m_className = cdes->GetName() + classNameSuffix;
  m_baseClassName = baseClassName;
}

////////////////////////////////////////////////////////////////////////////////

ClassStubGenerator::~ClassStubGenerator()
{
}

////////////////////////////////////////////////////////////////////////////////

void ClassStubGenerator::GenerateCode()
{
  GenerateHpp();
  GenerateCpp();
}

////////////////////////////////////////////////////////////////////////////////

void ClassStubGenerator::GenerateHpp()
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

void ClassStubGenerator::GenerateCpp()
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

void ClassStubGenerator::GenerateDefinition(ofstream& sortida)
{
  sortida << "#pragma once" << endl;
  sortida << endl;

  sortida << "#include \"" << m_baseClassName << ".h\"" << endl;
  sortida << endl;  

  sortida << "class " << m_className << " : public " << m_baseClassName << endl;
  sortida << "{" << endl;

  GenerateDefinitionMethods(sortida);
  GenerateDefinitionSpecificCode(sortida);
  
  sortida << "};" << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassStubGenerator::GenerateDefinitionMethods(ofstream& sortida)
{
  sortida << "public:" << endl;
  sortida << endl;

  sortida << "  IUnknown* GetIUnknown() const;" << endl;
  sortida << "  HRESULT HandleCall(DXMethodCallPtr call);" << endl;

  if (m_classSpecificCode)
  {
    vector<string>* v_names = m_classSpecificCode->GetMethodNames();

    for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
    {
      if (m_classSpecificCode->GetMethod(*it)->GetPolicy())
      {
        if (m_classSpecificCode->GetMethod(*it)->GetPolicy()->GetType() == MethodSpecificCode::ReplaceBoth || m_classSpecificCode->GetMethod(*it)->GetPolicy()->GetType() == MethodSpecificCode::ReplaceStubOnly)
        {
          sortida << "  HRESULT DoSpecific(DXMethodCallPtr call);" << endl;
          break;
        }
      }
    }

    delete v_names;
  }

  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassStubGenerator::GenerateDefinitionSpecificCode(ofstream& sortida)
{
  if (!m_classSpecificCode)
  {
    return;
  }

  sortida << "  //////////////////////////////////////////////////////////////////////////////" << endl;
  sortida << "  // BEGIN Specific Code" << endl;
  sortida << "  //////////////////////////////////////////////////////////////////////////////" << endl;
  sortida << endl;

  GenerateDefinitionSpecificCodeAttributes(sortida);
  GenerateDefinitionSpecificCodeMethodAdd(sortida);

  sortida << "  //////////////////////////////////////////////////////////////////////////////" << endl;
  sortida << "  // END Specific Code" << endl;
  sortida << "  //////////////////////////////////////////////////////////////////////////////" << endl;
  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassStubGenerator::GenerateDefinitionSpecificCodeAttributes(ofstream& sortida)
{
  sortida << "protected:" << endl;
  sortida << endl;

  vector<string>* v_names = m_classSpecificCode->GetStubAttributeNames();

  for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
  {
    sortida << "  " << m_classSpecificCode->GetStubAttribute(*it)->GetType() << " " << m_classSpecificCode->GetStubAttribute(*it)->GetName() << ";" << endl;
  }

  delete v_names;

  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassStubGenerator::GenerateDefinitionSpecificCodeMethodAdd(ofstream& sortida)
{
  vector<string> v_namesPublic;
  vector<string> v_namesProtected;

  vector<string>* v_names = m_classSpecificCode->GetStubNewMethodNames();
  for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
  {
    if (m_classSpecificCode->GetStubNewMethod(*it)->GetIsPublic())
      v_namesPublic.push_back(*it);
    else
      v_namesProtected.push_back(*it);
  }
  delete v_names;

  if (v_namesPublic.size() > 0)
  {
    sortida << "public:" << endl;
    sortida << endl;

    for (vector<string>::iterator it = v_namesPublic.begin(); it != v_namesPublic.end(); it++)
    {
      sortida << "  " << m_classSpecificCode->GetStubNewMethod(*it)->GetDefinition() << ";" << endl;
    }

    sortida << endl;
  }

  if (v_namesProtected.size() > 0)
  {
    sortida << "protected:" << endl;
    sortida << endl;

    for (vector<string>::iterator it = v_namesProtected.begin(); it != v_namesProtected.end(); it++)
    {
      sortida << "  " << m_classSpecificCode->GetStubNewMethod(*it)->GetDefinition() << ";" << endl;
    }

    sortida << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

void ClassStubGenerator::GenerateImplementation(ofstream& sortida)
{
  sortida << "#include \"" << m_className << ".h\"" << endl;
  sortida << endl;

  sortida << "IUnknown* " << m_className << "::GetIUnknown() const" << endl;
  sortida << "{" << endl;
  sortida << "  return (IUnknown*) m_original;" << endl;
  sortida << "}" << endl;
  sortida << endl;
  
  GenerateImplementationHandleCall(sortida);
}

////////////////////////////////////////////////////////////////////////////////

void ClassStubGenerator::GenerateImplementationHandleCall(ofstream& sortida)
{
  sortida << "HRESULT " << m_className << "::HandleCall(DXMethodCallPtr call)" << endl;
  sortida << "{" << endl;
  
  sortida << "  switch (call->GetToken())" << endl;
  sortida << "  {" << endl;
  
  for (unsigned int i=0, j=m_classDescription->GetMethodsCount(); i < j; i++)
  {
    sortida << "  case DXMethodCallHelper::TOK_" << m_classDescription->GetName() << "_" << m_classDescription->GetMethod(i)->GetName() << ":" << endl;
    
    bool doSpecific = false;
    if ((bool) m_classSpecificCode && (bool) m_classSpecificCode->GetMethod(m_classDescription->GetMethod(i)->GetName()) && (bool) m_classSpecificCode->GetMethod(m_classDescription->GetMethod(i)->GetName())->GetPolicy())
    {
      MethodSpecificCodePtr methodSC = m_classSpecificCode->GetMethod(m_classDescription->GetMethod(i)->GetName());
      if (methodSC->GetPolicy()->GetType() == MethodSpecificCode::ReplaceBoth || methodSC->GetPolicy()->GetType() == MethodSpecificCode::ReplaceStubOnly)
      {
        doSpecific = true;
      }
    }
    
    if (doSpecific)
    {
      sortida << "    return DoSpecific(call);" << endl;
    }
    else
    {
      GenerateImplementationHandleCallToken(sortida, m_classDescription->GetMethod(i));
    }

    sortida << "    break;" << endl;
    sortida << endl;
  }
  
  sortida << "  }" << endl;
  sortida << endl;
  
  sortida << "  return E_NOTIMPL;" << endl;
  sortida << "}" << endl;
  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassStubGenerator::GenerateImplementationHandleCallToken(ofstream& sortida, MethodDescriptionPtr method)
{
  MethodSpecificCodePtr methodSC = NULL;
  if (m_classSpecificCode)
  {
    methodSC = m_classSpecificCode->GetMethod(method->GetName());
  }
  
  sortida << "    {" << endl;
  
  ostringstream paramsCall;
  bool callSavedIncorrectly = false;
  for (unsigned int i=0, j=method->GetParamsCount(); i < j; i++)
  {
    MethodDescriptionParamPtr param = method->GetParam(i);
    unsigned int paramPosition = i+1;
    
    if (param->GetType().empty())
    {
      continue;
    }

    bool captureThisParam = true;
    if ((bool) methodSC && (bool) methodSC->GetParam(paramPosition))
    {
      if (methodSC->GetParam(paramPosition)->GetPolicy(ParamSpecificCode::NoCapture))
      {
        captureThisParam = false;
      }
    }
    
    if (captureThisParam)
    {
      string paramType = param->GetType();
      if ((bool) methodSC && (bool) methodSC->GetParam(paramPosition))
      {
        if (methodSC->GetParam(paramPosition)->GetPolicy(ParamSpecificCode::ChangeSaveType))
        {
          ParamSpecificCode::ChangeSaveTypePolicy* paramPolicy = (ParamSpecificCode::ChangeSaveTypePolicy*) (void*) methodSC->GetParam(paramPosition)->GetPolicy(ParamSpecificCode::ChangeSaveType);
          paramType = paramPolicy->GetSaveType();
        }
      }

      TypeSpecificCodePtr typeSC = m_config.GetTypeSpecificCode(paramType); 
      if (typeSC)
      {
        string useMethod = typeSC->GetUseMethod();
        useMethod.replace(0, 5, "Pop_");
        paramType = useMethod.substr(4, useMethod.size()-4);
        TypeSpecificCode::PassBy typePassBy = typeSC->GetPassBy();

        switch (typeSC->GetSaveIn())
        {
        case TypeSpecificCode::SI_CallStack:
          
          // Small hack to avoid to change a lot of things in the system
          if (paramType.compare("IID") == 0)
          {
            typePassBy = TypeSpecificCode::PB_Value;
          }
          
          switch (typePassBy)
          {
          case TypeSpecificCode::PB_Value:
            
            sortida << "      " << paramType << " " << param->GetName() << ";" << endl;
            sortida << "      CHECK_CALL(call->" << useMethod << "(&" << param->GetName() << "));" << endl;
            if (paramType.compare("HWND") == 0)
            {
              sortida << "      if (" << param->GetName() << ") " << param->GetName() << " = m_painter->GetViewportHWND();" << endl;
            }
            sortida << endl;
            
            paramsCall << ((unsigned int) paramsCall.tellp() != -1 ? ", " : "") << (param->GetType().substr(param->GetType().size()-1, 1).compare("*") == 0 ? "&" : "") << param->GetName();
            break;
          
          case TypeSpecificCode::PB_Reference:
            
            sortida << "      " << paramType << " " << param->GetName() << ";" << endl;
            sortida << "      " << paramType << "* " << param->GetName() << "Ptr = &" << param->GetName() << ";" << endl;
            sortida << "      {" << endl;
            sortida << "        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))" << endl;
            sortida << "        {" << endl;
            sortida << "          DXNullPointer checkNull;" << endl;
            sortida << "          call->Pop_DXNullPointer(&checkNull);" << endl;
            sortida << "          " << param->GetName() << "Ptr = NULL;" << endl;
            sortida << "        }" << endl;
            sortida << "        else" << endl;
            sortida << "        {" << endl;
            sortida << "          CHECK_CALL(call->" << useMethod << "(" << param->GetName() << "Ptr));" << endl;
            sortida << "        }" << endl;
            sortida << "      }" << endl;
            sortida << endl;
            
            paramsCall << ((unsigned int) paramsCall.tellp() != -1 ? ", " : "") << param->GetName() << "Ptr";
            break;
          }
          break;

        case TypeSpecificCode::SI_Buffer:
          
          sortida << "      char* " << param->GetName() << ";" << endl;
          sortida << "      DXBufferPtr " << param->GetName() << "Buffer;" << endl;
          sortida << "      {" << endl;
          sortida << "        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))" << endl;
          sortida << "        {" << endl;
          sortida << "          DXNullPointer checkNull;" << endl;
          sortida << "          call->Pop_DXNullPointer(&checkNull);" << endl;
          sortida << "          " << param->GetName() << " = NULL;" << endl;
          sortida << "        }" << endl;
          sortida << "        else" << endl;
          sortida << "        {" << endl;
          sortida << "          DXBufferIdentifier buffer_id;" << endl;
          sortida << "          CHECK_CALL(call->Pop_DXBufferIdentifier(&buffer_id));" << endl;
          sortida << "          CHECK_CALL(m_painter->GetBuffer(&" << param->GetName() << "Buffer, buffer_id));" << endl;
          sortida << "          CHECK_CALL(" << param->GetName() << "Buffer->" << useMethod << "(&" << param->GetName() << "));" << endl;
          sortida << "        }" << endl;
          sortida << "      }" << endl;
          sortida << endl;
          
          paramsCall << ((unsigned int) paramsCall.tellp() != -1 ? ", " : "") << "(" << param->GetType() << ") " << param->GetName();
          break;
        }
      }
      else
      {
        sortida << "      // CODEGEN_COMMENT: param '" << param->GetType() << " " << param->GetName() << "' not saved!" << endl;
        sortida << endl;
        callSavedIncorrectly = true;
      }
    }
    else
    {
      string paramType = param->GetType();
      
      // Small hack to avoid to change a lot of things in the system
      if (paramType.compare("void*") != 0)
      {
        paramType = paramType.substr(0, paramType.size()-1);
        paramsCall << ((unsigned int) paramsCall.tellp() != -1 ? ", " : "") << "&" << param->GetName();
        sortida << "      " << paramType << " " << param->GetName() << ";" << endl;
      }
      else
      {
        paramsCall << ((unsigned int) paramsCall.tellp() != -1 ? ", " : "") << param->GetName();
        sortida << "      " << paramType << " " << param->GetName() << " = NULL;" << endl;
      }

      sortida << "      {" << endl;
      sortida << "        DXIgnoredParameter ignoredParam;" << endl;
      sortida << "        CHECK_CALL(call->Pop_DXIgnoredParameter(&ignoredParam));" << endl;
      sortida << "      }" << endl;
      sortida << endl;
    }
  }
  
  if (!callSavedIncorrectly)
  {
    if (method->GetType().compare("void") == 0)
    {
      sortida << "      m_original->" << method->GetName() << "(" << paramsCall.str() << ");" << endl;
      sortida << "      return D3D_OK;" << endl;
    }
    else
    {
      TypeSpecificCodePtr typeSC = m_config.GetTypeSpecificCode(method->GetType()); 
      if (typeSC)
      {
        string methodName = typeSC->GetUseMethod();
        string methodType = methodName.substr(5, methodName.size()-5);
        sortida << "      " << methodType << " result = m_original->" << method->GetName() << "(" << paramsCall.str() << ");" << endl;
        if (methodType.compare("HRESULT") == 0)
          sortida << "      CHECK_CALL_RETURN_VALUE_HRESULT(result);" << endl;
        else
          sortida << "      CHECK_CALL_RETURN_VALUE(" << methodType << ", result);" << endl;
      }
      else
      {
        sortida << "      // CODEGEN_COMMENT: unknow method to extract type '" << method->GetType() << "' from the call!" << endl;
        sortida << "      return E_NOTIMPL;" << endl;
      }      
    }
  }
  else
  {
    sortida << "      // CODEGEN_COMMENT: don't call Direct3D because still unknow to save all the parameters!" << endl;
    sortida << "      return E_NOTIMPL;" << endl;
  }

  sortida << "    }" << endl;
}

////////////////////////////////////////////////////////////////////////////////
