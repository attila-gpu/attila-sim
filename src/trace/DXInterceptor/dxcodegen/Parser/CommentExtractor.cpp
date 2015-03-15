////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Parser/CommentExtractor.h"

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

CommentExtractor::~CommentExtractor()
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
