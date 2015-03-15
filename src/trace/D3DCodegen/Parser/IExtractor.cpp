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

#include "Parser/IExtractor.hpp"

using namespace std;
using namespace dxcodegen::Parser;

////////////////////////////////////////////////////////////////////////////////

IExtractor::IExtractor(string& cadena, bool verbose) :
m_cadena(cadena),
m_verbose(verbose)
{
}

////////////////////////////////////////////////////////////////////////////////

void IExtractor::Extract()
{
  MatchResults resultat;
  do {
    resultat.Clear();
      resultat = Match(m_cadena);
    if (resultat.matched)
    {
      Parse(resultat);
    }
  } while(resultat.matched);
}

////////////////////////////////////////////////////////////////////////////////
