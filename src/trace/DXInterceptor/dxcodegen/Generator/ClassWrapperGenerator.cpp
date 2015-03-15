////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Config/GeneratorConfiguration.h"
#include "Items/ClassSpecificCode.h"
#include "Items/ClassDescription.h"
#include "Generator/ClassWrapperGenerator.h"

using namespace std;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;
using namespace dxcodegen::Generator;

////////////////////////////////////////////////////////////////////////////////

ClassWrapperGenerator::ClassWrapperGenerator(GeneratorConfiguration& config, ClassDescriptionPtr cdes, const string& outputPath, const string& classNameSuffix, const string& baseClassName) :
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

ClassWrapperGenerator::~ClassWrapperGenerator()
{
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateCode()
{
  GenerateHpp();
  GenerateCpp();
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateHpp()
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

void ClassWrapperGenerator::GenerateCpp()
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

void ClassWrapperGenerator::GenerateMethodHeader(ofstream& sortida, MethodDescriptionPtr method, const string& altname, bool passMethodCallSaverInstance)
{
  if (altname.empty())
  {
    sortida << method->GetName();
  }
  else
  {
    sortida << altname;
  }
  sortida << "(";

  for (unsigned int i=0, j=method->GetParamsCount(); i < j; i++)
  {
    if (!method->GetParam(i)->GetType().empty())
    {
      sortida << method->GetParam(i)->GetType();
      sortida << " ";
      sortida << method->GetParam(i)->GetName();
      if (i+1 < j)
      {
        sortida << ", ";
      }
    }
    else
    {
      break;
    }
  }

  if (passMethodCallSaverInstance)
  {
    if (method->GetParamsCount() > 0 && !method->GetParam(0)->GetType().empty())
    {
      sortida << ", ";
    }
    sortida << "DXMethodCallPtr call";
  }

  sortida << ")";
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateMethodCall(ofstream& sortida, MethodDescriptionPtr method, const string& altname, bool passMethodCallSaverInstance)
{
  if (altname.empty())
  {
    sortida << method->GetName();
  }
  else
  {
    sortida << altname;
  }
  sortida << "(";
  
  for (unsigned int i=0, j=method->GetParamsCount(); i < method->GetParamsCount(); i++)
  {
    if (!method->GetParam(i)->GetType().empty())
    {
      sortida << method->GetParam(i)->GetName();
      if (i+1 < j)
      {
        sortida << ", ";
      }
    }
    else
    {
      break;
    }
  }
  
  if (passMethodCallSaverInstance)
  {
    if (method->GetParamsCount() > 0 && !method->GetParam(0)->GetType().empty())
    {
      sortida << ", ";
    }
    sortida << "call";
  }
  
  sortida << ")";
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateDefinition(ofstream& sortida)
{
  sortida << "#pragma once" << endl;
  sortida << endl;

  sortida << "#include \"" << m_baseClassName << ".h\"" << endl;
  sortida << endl;  
  
  sortida << "interface " << m_className << " : public " << m_classDescription->GetName() << ", public " << m_baseClassName << endl;
  sortida << "{" << endl;
  
  GenerateDefinitionMethods(sortida);
  GenerateDefinitionSpecificCode(sortida);

  sortida << "};" << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateDefinitionMethods(ofstream& sortida)
{
  sortida << "public:" << endl;
  sortida << endl;

  for (unsigned int i=0; i < m_classDescription->GetMethodsCount(); i++)
  {
    GenerateDefinitionMethod(sortida, m_classDescription->GetMethod(i));
  }

  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateDefinitionMethod(ofstream& sortida, MethodDescriptionPtr method)
{
  sortida << "  virtual ";
  sortida << method->GetType();
  sortida << " __stdcall ";

  GenerateMethodHeader(sortida, method);

  sortida << ";" << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateDefinitionSpecificCode(ofstream& sortida)
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
  GenerateDefinitionSpecificCodeMethodReplace(sortida);
    
  sortida << "  //////////////////////////////////////////////////////////////////////////////" << endl;
  sortida << "  // END Specific Code" << endl;
  sortida << "  //////////////////////////////////////////////////////////////////////////////" << endl;
  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateDefinitionSpecificCodeAttributes(ofstream& sortida)
{
  sortida << "protected:" << endl;
  sortida << endl;

  vector<string>* v_names = m_classSpecificCode->GetWrapperAttributeNames();

  for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
  {
    sortida << "  " << m_classSpecificCode->GetWrapperAttribute(*it)->GetType() << " " << m_classSpecificCode->GetWrapperAttribute(*it)->GetName() << ";" << endl;
  }

  delete v_names;

  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateDefinitionSpecificCodeMethodAdd(ofstream& sortida)
{
  vector<string> v_namesPublic;
  vector<string> v_namesProtected;
  
  vector<string>* v_names = m_classSpecificCode->GetWrapperNewMethodNames();
  for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
  {
    if (m_classSpecificCode->GetWrapperNewMethod(*it)->GetIsPublic())
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
      sortida << "  " << m_classSpecificCode->GetWrapperNewMethod(*it)->GetDefinition() << ";" << endl;
    }

    sortida << endl;
  }

  if (v_namesProtected.size() > 0)
  {
    sortida << "protected:" << endl;
    sortida << endl;

    for (vector<string>::iterator it = v_namesProtected.begin(); it != v_namesProtected.end(); it++)
    {
      sortida << "  " << m_classSpecificCode->GetWrapperNewMethod(*it)->GetDefinition() << ";" << endl;
    }

    sortida << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateDefinitionSpecificCodeMethodReplace(ofstream& sortida)
{
  sortida << "protected:" << endl;
  sortida << endl;

  vector<string>* v_names = m_classSpecificCode->GetMethodNames();
  
  for (vector<string>::iterator it = v_names->begin(); it != v_names->end(); it++)
  {
    if ((bool) m_classDescription->GetMethod(*it) && (bool) m_classSpecificCode->GetMethod(*it)->GetPolicy())
    {
      MethodDescriptionPtr method = m_classDescription->GetMethod(*it);
      MethodSpecificCodePtr methodSC = m_classSpecificCode->GetMethod(*it);

      if (methodSC->GetPolicy()->GetType() != MethodSpecificCode::ReplaceBoth && methodSC->GetPolicy()->GetType() != MethodSpecificCode::ReplaceWrapperOnly)
      {
        continue;
      }

      string methodReplaceName = "";
      bool passCallSaver = false;
      
      switch (methodSC->GetPolicy()->GetType())
      {
      case MethodSpecificCode::ReplaceBoth:
        {
          MethodSpecificCode::ReplaceBothPolicy* policy = (MethodSpecificCode::ReplaceBothPolicy*) (void*) methodSC->GetPolicy();
          methodReplaceName = policy->GetMethodName();
          passCallSaver = policy->GetPassCallSaver();
        }
        break;

      case MethodSpecificCode::ReplaceWrapperOnly:
        {
          MethodSpecificCode::ReplaceWrapperOnlyPolicy* policy = (MethodSpecificCode::ReplaceWrapperOnlyPolicy*) (void*) methodSC->GetPolicy();
          methodReplaceName = policy->GetMethodName();
          passCallSaver = policy->GetPassCallSaver();
        }
        break;
      }

      sortida << "  ";
      sortida << method->GetType();
      sortida << " ";
      GenerateMethodHeader(sortida, method, methodReplaceName, passCallSaver);
      sortida << ";" << endl;
    }
  }

  delete v_names;
  
  sortida << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateImplementation(ofstream& sortida)
{
  sortida << "#include \"" << m_className << ".h\"" << endl;
  sortida << endl;
  
  GenerateImplementationMethods(sortida);
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateImplementationMethods(ofstream& sortida)
{
  for (unsigned int i=0, j=m_classDescription->GetMethodsCount(); i < j; i++)
  {
    GenerateImplementationMethod(sortida, m_classDescription->GetMethod(i));

    if (i+1 < j)
    {
      sortida << endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateImplementationMethod(ofstream& sortida, MethodDescriptionPtr method)
{
  sortida << method->GetType();
  sortida << " ";
  sortida << m_className << "::";
  
  GenerateMethodHeader(sortida, method);

  sortida << endl;
  sortida << "{" << endl;

  GenerateImplementationMethodBodySaveCall(sortida, method);

  sortida << "}" << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateImplementationMethodBodyCall(ofstream& sortida, MethodDescriptionPtr method)
{
  sortida << "  ";
  if (method->GetType() != "void")
  {
    sortida << method->GetType() << " result = ";
  }
  
  bool doSpecificCode = false;
  MethodSpecificCodePtr methodSC = NULL;

  if (m_classSpecificCode)
  {
    methodSC = m_classSpecificCode->GetMethod(method->GetName());
    if ((bool) methodSC && (bool) methodSC->GetPolicy())
    {
      doSpecificCode = (methodSC->GetPolicy()->GetType() == MethodSpecificCode::ReplaceBoth || methodSC->GetPolicy()->GetType() == MethodSpecificCode::ReplaceWrapperOnly);
    }
  }

  if (doSpecificCode)
  {
    switch (methodSC->GetPolicy()->GetType())
    {
    case MethodSpecificCode::ReplaceBoth:
      {
        MethodSpecificCode::ReplaceBothPolicy* policy = (MethodSpecificCode::ReplaceBothPolicy*) (void*) methodSC->GetPolicy();
        GenerateMethodCall(sortida, method, policy->GetMethodName(), policy->GetPassCallSaver());
      }
      break;
    
    case MethodSpecificCode::ReplaceWrapperOnly:
      {
        MethodSpecificCode::ReplaceWrapperOnlyPolicy* policy = (MethodSpecificCode::ReplaceWrapperOnlyPolicy*) (void*) methodSC->GetPolicy();
        GenerateMethodCall(sortida, method, policy->GetMethodName(), policy->GetPassCallSaver());
      }
      break;
    }
  }
  else
  {
    sortida << "m_original->";
    GenerateMethodCall(sortida, method);
  }
  
  sortida << ";" << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ClassWrapperGenerator::GenerateImplementationMethodBodySaveCall(ofstream& sortida, MethodDescriptionPtr method)
{
  sortida << "#ifdef D3D_MULTITHREAD_SUPPORT" << endl;
  sortida << "  EnterCriticalSection(&gCriticalSection);" << endl;
  sortida << "#endif // ifdef D3D_MULTITHREAD_SUPPORT" << endl;
  sortida << endl;
  
  sortida << "#ifdef WRITE_METHOD_CALLS_NAMES_TO_LOG" << endl;
  sortida << "  g_logger->Write(\"" <<  m_classDescription->GetName() << "[%u]::" << method->GetName() << "()\", GetObjectID());" << endl;
  sortida << "#endif // ifdef WRITE_METHOD_CALLS_NAMES_TO_LOG" << endl;
  sortida << endl;
  
  sortida << "  DXMethodCallPtr call = new DXMethodCall(DXMethodCallHelper::TOK_" << m_classDescription->GetName() << "_" << method->GetName() << ", m_objectID);" << endl;

  bool writedMethodCall = false;
  for (unsigned int i=0, j=method->GetParamsCount(); i < j; i++)
  {
    if (method->GetParam(i)->GetType().empty())
    {
      break;
    }
    
    if (GenerateImplementationMethodBodySaveCallParam(sortida, method, i+1))
    {
      if (!writedMethodCall)
      {
        sortida << endl;
        GenerateImplementationMethodBodyCall(sortida, method);
        sortida << endl;
        
        writedMethodCall = true;
      }
    }
  }

  if (!writedMethodCall)
  {
    sortida << endl;
    GenerateImplementationMethodBodyCall(sortida, method);
    sortida << endl;
  }

  if (method->GetType() != "void")
  {
    TypeSpecificCodePtr typeSC = m_config.GetTypeSpecificCode(method->GetType()); 
    if (typeSC)
    {
      sortida << "  // Save the return value" << endl;
      if (method->GetType() != "HRESULT")
      {
        sortida << "  call->" << typeSC->GetUseMethod() << "(result);" << endl;
        sortida << "  call->SetIsSavedReturnValue(true);" << endl;
      }
      else
      {
        sortida << "  if (result != D3D_OK)" << endl;
        sortida << "  {" << endl;
        sortida << "    call->" << typeSC->GetUseMethod() << "(result);" << endl;
        sortida << "    call->SetIsSavedReturnValue(true);" << endl;
        sortida << "  }" << endl;
      }
      sortida << endl;
    }
    else
    {
      sortida << "  // CODEGEN_COMMENT: unknow method to save the return value!" << endl;
      sortida << "  call->Push_int(-1);" << endl;
      sortida << "  call->SetIsSavedReturnValue(true);" << endl;
      sortida << endl;
    }
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Process statistics from this method call and save the call
  
  sortida << "  // Save statistics" << endl;
  sortida << "  if (g_statman)" << endl;
  sortida << "  {" << endl;
  sortida << "    g_statman->ProcessCall(call);" << endl;
  sortida << "    g_traceman->WriteMethodCall(call);" << endl;

  if ((m_classDescription->GetName().compare("IDirect3DDevice9")    == 0 && method->GetName().compare("Present") == 0) ||
      (m_classDescription->GetName().compare("IDirect3DSwapChain9") == 0 && method->GetName().compare("Present") == 0))
  {
    sortida << endl;
    sortida << "    // Save end frame statistics" << endl;
    sortida << "    DXStatisticPtr stats = new DXStatistic();" << endl;
    sortida << "    if (g_statman->GetStatisticsFrame(stats))" << endl;
    sortida << "    {" << endl;
    sortida << "      unsigned int statistic_id = 0;" << endl;
    sortida << "      g_traceman->WriteStatistic(stats, statistic_id);" << endl;
    sortida << "    }" << endl;
  }

  sortida << "  }" << endl;
  sortida << "  else" << endl;
  sortida << "  {" << endl;
  sortida << "    g_traceman->WriteMethodCall(call);" << endl;
  sortida << "  }" << endl;
  sortida << endl;
  
  //////////////////////////////////////////////////////////////////////////////
  
  if (method->GetName() != "Release")
  {
    sortida << "#ifdef D3D_MULTITHREAD_SUPPORT" << endl;
    sortida << "  LeaveCriticalSection(&gCriticalSection);" << endl;
    sortida << "#endif // ifdef D3D_MULTITHREAD_SUPPORT" << endl;
    sortida << endl;
    
    if (method->GetType() != "void")
    {
      sortida << "  return result;" << endl;
    }
  }
  else
  {
    sortida << "  if (!result)" << endl;
    sortida << "  {" << endl;
    sortida << "    delete this;" << endl;
    sortida << "  }" << endl;
    
    sortida << endl;
    sortida << "#ifdef D3D_MULTITHREAD_SUPPORT" << endl;
    sortida << "  LeaveCriticalSection(&gCriticalSection);" << endl;
    sortida << "#endif // ifdef D3D_MULTITHREAD_SUPPORT" << endl;
    sortida << endl;
    
    sortida << "  return result;" << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool ClassWrapperGenerator::GenerateImplementationMethodBodySaveCallParam(ofstream& sortida, MethodDescriptionPtr method, unsigned int paramPosition)
{
  MethodSpecificCodePtr methodSC = NULL;
  if (m_classSpecificCode)
  {
    methodSC = m_classSpecificCode->GetMethod(method->GetName());
  }
  
  bool captureThisParam = true;
  if ((bool) methodSC && (bool) methodSC->GetParam(paramPosition))
  {
    if (methodSC->GetParam(paramPosition)->GetPolicy(ParamSpecificCode::SaveSpecific))
    {
      return true;
    }
    
    if (methodSC->GetParam(paramPosition)->GetPolicy(ParamSpecificCode::NoCapture))
    {
      captureThisParam = false;
    }
  }

  if (captureThisParam)
  {
    MethodDescriptionParamPtr param = method->GetParam(paramPosition-1);
    
    string paramType = param->GetType();
    string helperParams;
    if ((bool) methodSC && (bool) methodSC->GetParam(paramPosition))
    {
      if (methodSC->GetParam(paramPosition)->GetPolicy(ParamSpecificCode::ChangeSaveType))
      {
        ParamSpecificCode::ChangeSaveTypePolicy* paramPolicy = (ParamSpecificCode::ChangeSaveTypePolicy*) (void*) methodSC->GetParam(paramPosition)->GetPolicy(ParamSpecificCode::ChangeSaveType);
        paramType = paramPolicy->GetSaveType();
      }
      
      if (methodSC->GetParam(paramPosition)->GetPolicy(ParamSpecificCode::HelpParams))
      {
        ostringstream str;
        ParamSpecificCode::HelpParamsPolicy* paramPolicy = (ParamSpecificCode::HelpParamsPolicy*) (void*) methodSC->GetParam(paramPosition)->GetPolicy(ParamSpecificCode::HelpParams);
        vector<unsigned int>& listParams = paramPolicy->GetHelpParamList();
        for (vector<unsigned int>::iterator it=listParams.begin(); it != listParams.end(); it++)
        {
          if ((bool) method->GetParam(((unsigned int) *it) - 1))
          {
            str << ", " << method->GetParam(((unsigned int) *it) - 1)->GetName();
          }
        }
        helperParams = str.str();
      }
    }
    
    TypeSpecificCodePtr typeSC = m_config.GetTypeSpecificCode(paramType);
    if (typeSC)
    {
      switch (typeSC->GetSaveIn())
      {
      case TypeSpecificCode::SI_CallStack:
        sortida << "  call->" << typeSC->GetUseMethod() << "(";
        switch (typeSC->GetPassBy())
        {
        case TypeSpecificCode::PB_Value:
          sortida << (param->GetType().substr(param->GetType().size()-1, 1).compare("*") == 0 ? "*" : "") << param->GetName();
          break;
        case TypeSpecificCode::PB_Reference:
          sortida << (param->GetType().substr(param->GetType().size()-1, 1).compare("*") == 0 ? "" : "&") << param->GetName();
          break;
        }
        sortida << helperParams;
        sortida << ");" << endl;
        break;

      case TypeSpecificCode::SI_Buffer:
        sortida << "  if (" << param->GetName() << " != NULL)" << endl;
        sortida << "  {" << endl;
        sortida << "    DXBufferIdentifier buffer_id;" << endl;
        sortida << "    DXBufferPtr buffer = new DXBuffer();" << endl;
        sortida << "    buffer->" << typeSC->GetUseMethod() << "(";
        switch (typeSC->GetPassBy())
        {
        case TypeSpecificCode::PB_Value:
          sortida << param->GetName();
          break;
        case TypeSpecificCode::PB_Reference:
          sortida << "&" << param->GetName();
          break;
        }
        sortida << helperParams;
        sortida << ");" << endl;
        sortida << "    g_traceman->WriteBuffer(buffer, buffer_id);" << endl;
        sortida << "    call->Push_DXBufferIdentifier(buffer_id);" << endl;;
        sortida << "  }" << endl;
        sortida << "  else" << endl;
        sortida << "  {" << endl;
        sortida << "    call->Push_DXNullPointer();" << endl;
        sortida << "  }" << endl;          
        break;
      }
    }

    //////////////////////////////////////////////////////////////////////////
    // Unknow type parameters
    //////////////////////////////////////////////////////////////////////////

    else
    {
      sortida << "  // CODEGEN_COMMENT: param '" << param->GetType() << " " << param->GetName() << "' not saved!" << endl;
      sortida << "  call->Push_int(-1);" << endl;
    }
  }
  else
  {
    sortida << "  call->Push_DXIgnoredParameter();" << endl;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
