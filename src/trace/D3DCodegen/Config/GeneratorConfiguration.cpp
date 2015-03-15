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

#include "Config/GeneratorConfiguration.hpp"

using namespace std;
using namespace dxcodegen::Config;

////////////////////////////////////////////////////////////////////////////////

void GeneratorConfiguration::SetOutputPath(const string& path)
{
  m_outputPath = path;
}

////////////////////////////////////////////////////////////////////////////////

string& GeneratorConfiguration::GetOutputPath()
{
  return m_outputPath;
}

////////////////////////////////////////////////////////////////////////////////
