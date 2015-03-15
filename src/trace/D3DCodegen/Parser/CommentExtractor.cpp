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

#include "Parser/CommentExtractor.hpp"

using namespace std;
using namespace regex;
using namespace dxcodegen::Parser;

////////////////////////////////////////////////////////////////////////////////

const rpattern patro_comment_single("(\\s*//.*)$", "", MULTILINE | GLOBAL | NOBACKREFS);
const rpattern patro_comment_multi("\\s*/\\*.*?\\*/", "", SINGLELINE | GLOBAL | NOBACKREFS);

////////////////////////////////////////////////////////////////////////////////

CommentExtractor::CommentExtractor(string& cadena) :
IExtractor(cadena)
{
}

////////////////////////////////////////////////////////////////////////////////

CommentExtractor::MatchResults CommentExtractor::Match(string& cadena)
{
  MatchResults resultat;
  subst_results results;
  
  // Match single line comment
  
  if (patro_comment_single.substitute(cadena, results) > 0)
  {
    resultat.matched = true;
    resultat.text.clear();
    return resultat;
  }

  // Match multiline comment

  if (patro_comment_multi.substitute(cadena, results) > 0)
  {
    resultat.matched = true;
    resultat.text.clear();
    return resultat;
  }

  return resultat;
}

////////////////////////////////////////////////////////////////////////////////

void CommentExtractor::Parse(const CommentExtractor::MatchResults& resultat)
{
}

////////////////////////////////////////////////////////////////////////////////
