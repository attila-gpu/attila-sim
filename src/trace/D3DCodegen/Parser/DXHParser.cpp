/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXCodeGenException.hpp"
#include "Utilities/Debug.hpp"
#include "Utilities/FileSystem.hpp"
#include "Utilities/String.hpp"
#include "Config/ParserConfiguration.hpp"
#include "Items/CppMacro.hpp"
#include "Items/EnumDescription.hpp"
#include "Items/StructDescription.hpp"
#include "Items/ClassDescription.hpp"
#include "Items/MethodDescription.hpp"
#include "Parser/CommentExtractor.hpp"
#include "Parser/PreprocessorExtractor.hpp"
#include "Parser/StructExtractor.hpp"
#include "Parser/EnumExtractor.hpp"
#include "Parser/ClassExtractor.hpp"
#include "Parser/MacroExpander.hpp"
#include "Parser/DXHParser.hpp"

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
  m_lstEnums = new vector<EnumDescription>;
  m_lstStructs = new vector<StructDescription>;
  m_lstClasses = new vector<ClassDescription>;
}

////////////////////////////////////////////////////////////////////////////////

DXHParser::~DXHParser()
{
  delete m_lstEnums;
  delete m_lstStructs;
  delete m_lstClasses;
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
    // PERFER [
    // Utilities::Debug::Print(*m_lstEnums);
    // Utilities::Debug::Print(*m_lstStructs);
    // Utilities::Debug::Print(*m_lstClasses);
    // ]
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
    ifstream entrada(filename.c_str(), ios_base::binary);

    entrada.seekg(0, ios_base::end);
    streampos size = entrada.tellg();
    entrada.seekg(0, ios_base::beg);

    char* buffer = new char[size];

    entrada.read(buffer, size);
    entrada.close();

    cadena.clear();
    cadena.assign(buffer, size);

    delete[] buffer;
}

////////////////////////////////////////////////////////////////////////////////

void DXHParser::AddEnums(vector<EnumDescription>& enums)
{
  vector<EnumDescription>::iterator it;
  for (it=enums.begin(); it != enums.end(); it++)
  {
    m_lstEnums->push_back(*it);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXHParser::AddStructs(vector<StructDescription>& structs)
{
  vector<StructDescription>::iterator it;
  for (it=structs.begin(); it != structs.end(); it++)
  {
    m_lstStructs->push_back(*it);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXHParser::AddClasses(vector<ClassDescription>& classes)
{
  vector<ClassDescription>::iterator it;
  for (it=classes.begin(); it != classes.end(); it++)
  {
    m_lstClasses->push_back(*it);
  }
}

////////////////////////////////////////////////////////////////////////////////

vector<ClassDescription>& DXHParser::GetClasses()
{
  return *m_lstClasses;
}

////////////////////////////////////////////////////////////////////////////////

vector<EnumDescription>& DXHParser::GetEnums()
{
  return *m_lstEnums;
}

////////////////////////////////////////////////////////////////////////////////

vector<StructDescription>& DXHParser::GetStructs()
{
  return *m_lstStructs;
}

////////////////////////////////////////////////////////////////////////////////
