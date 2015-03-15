////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXCodeGenException.h"
#include "Utilities/Debug.h"
#include "Utilities/FileSystem.h"
#include "Utilities/String.h"
#include "Config/ParserConfiguration.h"
#include "Items/CppMacro.h"
#include "Items/EnumDescription.h"
#include "Items/StructDescription.h"
#include "Items/ClassDescription.h"
#include "Parser/CommentExtractor.h"
#include "Parser/PreprocessorExtractor.h"
#include "Parser/StructExtractor.h"
#include "Parser/EnumExtractor.h"
#include "Parser/ClassExtractor.h"
#include "Parser/MacroExpander.h"
#include "Parser/DXHParser.h"

using namespace std;
using namespace dxcodegen;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;
using namespace dxcodegen::Parser;

////////////////////////////////////////////////////////////////////////////////

DXHParser::DXHParser(ParserConfiguration& config, bool verbose) :
m_config(config),
m_verbose(verbose)
{
}

////////////////////////////////////////////////////////////////////////////////

DXHParser::~DXHParser()
{
}

////////////////////////////////////////////////////////////////////////////////

void DXHParser::ParseFiles()
{	
	try
	{
    vector<string>::iterator it;
	  for (it=m_config.GetFiles().begin(); it != m_config.GetFiles().end(); it++)
	  {
		  ParseFile(*it);
	  }
	}
	catch (...)
	{
		throw;
	}
}

////////////////////////////////////////////////////////////////////////////////

void DXHParser::ParseFile(const string& filename)
{
  if (!Utilities::FileSystem::FilenameExists(filename))
  {
    DXCodeGenException e("could'nt open file '" + filename + "'");
    throw e;
  } 
  
  string cadena;
	
  ReadFileData(filename, cadena);

  CommentExtractor* eComment = new CommentExtractor(cadena);
  eComment->Extract();
  delete eComment;
  
  PreprocessorExtractor* ePreprocessor = new PreprocessorExtractor(cadena);
  ePreprocessor->Extract();
  delete ePreprocessor;

  StructExtractor* eStruct = new StructExtractor(m_config, cadena);
  eStruct->Extract();
  AddStructs(eStruct->GetStructs());
  delete eStruct;
  
  EnumExtractor* eEnum = new EnumExtractor(m_config, cadena);
  eEnum->Extract();
  AddEnums(eEnum->GetEnums());
  delete eEnum;
  
  MacroExpander* eMacro = new MacroExpander(m_config.GetMacros(), false);
  eMacro->Expand(cadena);
  delete eMacro;
  
  ClassExtractor* eClass = new ClassExtractor(m_config, cadena);
  eClass->Extract();
  AddClasses(eClass->GetClasses());
  delete eClass;
}

////////////////////////////////////////////////////////////////////////////////

void DXHParser::ReadFileData(const string& filename, string& cadena)
{
	ifstream entrada(filename.c_str(), ios::binary);

	entrada.seekg(0, ios::end);
	streampos size = entrada.tellg();
	entrada.seekg(0, ios::beg);

	char* buffer = new char[size];

	entrada.read(buffer, size);
	entrada.close();

	cadena.clear();
	cadena.assign(buffer, size);

	delete[] buffer;
}

////////////////////////////////////////////////////////////////////////////////

void DXHParser::AddEnums(const vector<EnumDescriptionPtr>& enums)
{
  vector<EnumDescriptionPtr>::const_iterator it;
  for (it=enums.begin(); it != enums.end(); it++)
  {
    m_lstEnums.push_back(*it);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXHParser::AddStructs(const vector<StructDescriptionPtr>& structs)
{
  vector<StructDescriptionPtr>::const_iterator it;
  for (it=structs.begin(); it != structs.end(); it++)
  {
    m_lstStructs.push_back(*it);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXHParser::AddClasses(const vector<ClassDescriptionPtr>& classes)
{
  vector<ClassDescriptionPtr>::const_iterator it;
  for (it=classes.begin(); it != classes.end(); it++)
  {
    m_lstClasses.push_back(*it);
  }
}

////////////////////////////////////////////////////////////////////////////////

const vector<ClassDescriptionPtr>& DXHParser::GetClasses()
{
  return m_lstClasses;
}

////////////////////////////////////////////////////////////////////////////////

const vector<EnumDescriptionPtr>& DXHParser::GetEnumerations()
{
  return m_lstEnums;
}

////////////////////////////////////////////////////////////////////////////////

const vector<StructDescriptionPtr>& DXHParser::GetStructures()
{
  return m_lstStructs;
}

////////////////////////////////////////////////////////////////////////////////
