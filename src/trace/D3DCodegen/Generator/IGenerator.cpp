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

#include "Generator/IGenerator.hpp"

using namespace std;
using namespace dxcodegen::Generator;

////////////////////////////////////////////////////////////////////////////////

void IGenerator::SetHeaderComment(const string& message)
{
  m_headerComment = message;
}

////////////////////////////////////////////////////////////////////////////////

string& IGenerator::GetHeaderCommment()
{
  return m_headerComment;
}

////////////////////////////////////////////////////////////////////////////////

ofstream* IGenerator::CreateFilename(const string& filename)
{
  ofstream* sortida = new ofstream(filename.c_str(), ios::out);
  if (sortida)
  {
    WriteHeaderComment(sortida);
  }
  return sortida;
}

////////////////////////////////////////////////////////////////////////////////

void IGenerator::WriteHeaderComment(ofstream* of)
{
  if (of && of->is_open())
  {
    *of << m_headerComment;
  }
}

////////////////////////////////////////////////////////////////////////////////

void IGenerator::CloseFilename(ofstream* of)
{
  if (of && of->is_open())
  {
    of->close();
    delete of;
  }
}

////////////////////////////////////////////////////////////////////////////////
