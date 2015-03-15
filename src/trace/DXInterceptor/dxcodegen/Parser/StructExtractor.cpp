////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utilities/String.h"
#include "Config/ParserConfiguration.h"
#include "Items/StructDescription.h"
#include "Parser/DXHParser.h"
#include "Parser/StructExtractor.h"

using namespace std;
using namespace regex;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;
using namespace dxcodegen::Parser;

////////////////////////////////////////////////////////////////////////////////

#define TYPEDEF_STRUCT 1
#define STRUCT 2

////////////////////////////////////////////////////////////////////////////////

const rpattern patro_typedef_struct("typedef \\s+ struct \\s+ (\\w+)? \\s* { \\s* (.*?) \\s* } \\s* ([^;]+) ;", EXTENDED | SINGLELINE);
const rpattern patro_typedef_struct_names_split(",");
const rpattern patro_typedef_struct_names_clean("\\s+", "", GLOBAL | NOBACKREFS);
const rpattern patro_struct("struct \\s+ (\\w+) \\s* { \\s* (.*?) \\s* } \\s* [^;]* ;", EXTENDED | SINGLELINE);
const rpattern patro_struct_members("(.*?);", SINGLELINE | GLOBAL | ALLBACKREFS);
const rpattern patro_struct_members_test("(.*?);", "", SINGLELINE | GLOBAL | NOBACKREFS);
const rpattern patro_struct_member("^(.*?)((\\w+)(\\s*(\\[[ \\d]+\\]|\\[[ \\w]+\\]))?)$", GLOBAL | ALLBACKREFS);
const rpattern patro_struct_member_test("^(.*?)((\\w+)(\\s*(\\[[ \\d]+\\]|\\[[ \\w]+\\]))?)$", "", GLOBAL | NOBACKREFS);
const rpattern patro_struct_member_clean("\\s+", " ", GLOBAL | NOBACKREFS);

////////////////////////////////////////////////////////////////////////////////

StructExtractor::StructExtractor(ParserConfiguration& config, string& cadena) :
m_config(config),
IExtractor(cadena)
{
}

////////////////////////////////////////////////////////////////////////////////

StructExtractor::~StructExtractor()
{
}

////////////////////////////////////////////////////////////////////////////////

const vector<StructDescriptionPtr>& StructExtractor::GetStructs()
{
  return m_lstStructs;
}

////////////////////////////////////////////////////////////////////////////////

StructExtractor::MatchResults StructExtractor::Match(string& cadena)
{
  MatchResults resultat;
  
  resultat = MatchTypedefStruct(cadena);
  if (resultat.matched)
  {
    return resultat;
  }

  resultat = MatchStruct(cadena);
  if (resultat.matched)
  {
    return resultat;
  }
  
  return resultat;
}

////////////////////////////////////////////////////////////////////////////////

StructExtractor::MatchResults StructExtractor::MatchTypedefStruct(string& cadena)
{
  MatchResults resultat;

  match_results results;
  match_results::backref_type br = patro_typedef_struct.match(cadena, results);
  if (br.matched)
  {
    resultat.matched = true;
    resultat.type = TYPEDEF_STRUCT;
    resultat.text = results.backref(0).str();
    cadena.erase(results.rstart(0), results.rlength(0));
  }

  return resultat;
}

////////////////////////////////////////////////////////////////////////////////

StructExtractor::MatchResults StructExtractor::MatchStruct(string& cadena)
{
  MatchResults resultat;

  match_results results;
  match_results::backref_type br = patro_struct.match(cadena, results);
  if (br.matched)
  {
    resultat.matched = true;
    resultat.type = STRUCT;
    resultat.text = results.backref(0).str();
    cadena.erase(results.rstart(0), results.rlength(0));
  }

  return resultat;
}

////////////////////////////////////////////////////////////////////////////////

void StructExtractor::Parse(const StructExtractor::MatchResults& resultat)
{
  switch (resultat.type)
  {
  case TYPEDEF_STRUCT:
    ParseTypedefStruct(resultat);
    break;

  case STRUCT:
    ParseStruct(resultat);
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void StructExtractor::ParseTypedefStruct(const StructExtractor::MatchResults& resultat)
{
  match_results results;
  patro_typedef_struct.match(resultat.text, results);

  string name = results.backref(3).str();
  string subcadena = results.backref(2).str();

  // Subdividim la llista de noms del typedef per tractar-los cadascun per
  // separat [
  split_results results_split;
  size_t num = patro_typedef_struct_names_split.split(name, results_split);
  // ]

  split_results::string_vector vec_names = results_split.strings();
  for (size_t i=0; i < vec_names.size(); i++)
  {
    name = vec_names[i];

    // Netejem el nom de possibles espais en blanc [
    subst_results results_clean;
    patro_typedef_struct_names_clean.substitute(name, results_clean);
    // ]

    if (m_config.IsStructParseCandidate(name))
    {
      if (m_verbose)
      {
        cout << "Parsing typedef struct " << name << endl;
      }
      ParseStructMembers(name, subcadena);
    }
    else
    {
      if (m_verbose)
      {
        cout << "Parsing typedef struct " << name << " -> IGNORED!" << endl;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void StructExtractor::ParseStruct(const StructExtractor::MatchResults& resultat)
{
  match_results results;
  patro_struct.match(resultat.text, results);

  string name = results.backref(1).str();
  string subcadena = results.backref(2).str();

  if (m_config.IsStructParseCandidate(name))
  {
    if (m_verbose)
    {
      cout << "Parsing struct " << name << endl;
    }
    ParseStructMembers(name, subcadena);
  }
  else
  {
    if (m_verbose)
    {
      cout << "Parsing struct " << name << " -> IGNORED!" << endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void StructExtractor::ParseStructMembers(const string& name, const string& cadena)
{	
  match_results results;
  match_results::backref_type br = patro_struct_members.match(cadena, results);
  if (br.matched)
  {
    // Eliminem de la cadena (amb tots els membres del struct) els elements
    // trobats. Si després de fer això encara queda algún caràcter en la cadena
    // significa que no l'hem parsejada totalment i per tant hi ha errors.
    // [		
    string test(cadena);
    subst_results results_test;
    patro_struct_members_test.substitute(test, results_test);
    Utilities::String::TrimString(test);
    // ]

    if (test.length() == 0)
    {
      StructDescriptionPtr sdes = new StructDescription();
      sdes->SetName(name);

      bool problem = false;
      for (size_t i=0; i < results.cbackrefs() && !problem; i+=2)
      {
        string member(results.backref(i+1).str());
        problem = !ParseStructMember(sdes, member);
      }

      if (!problem)
      {
        m_lstStructs.push_back(sdes);
      }
    }
    else
    {
      cout << "PARSE ERROR: [struct " << name << "] not all struct members processed!" << endl;
      cout << "> already remain in the string '" << test << "'" << endl;
    }
  }
  else
  {
    cout << "PARSE ERROR: [struct " << name << "] no members found!" << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool StructExtractor::ParseStructMember(StructDescriptionPtr sdes, string& cadena)
{
  // Netejem la cadena de possibles salts de línia i espais en blanc [
  subst_results results_clean;
  patro_struct_member_clean.substitute(cadena, results_clean);
  Utilities::String::TrimString(cadena);
  // ]

  match_results results;
  match_results::backref_type br = patro_struct_member.match(cadena, results);
  if (br.matched)
  {
    // Eliminem de la cadena (amb un membre del struct) els elements trobats
    // Si després de fer això encara queda algún caràcter en la cadena significa
    // que no l'hem parsejada totalment i per tant hi ha errors.
    // [		
    string test(cadena);
    subst_results results_test;
    patro_struct_member_test.substitute(test, results_test);
    Utilities::String::TrimString(test);
    // ]

    if (test.length() == 0)
    {
      StructDescriptionFieldPtr sfield = new StructDescriptionField();

      string type = results.backref(1).str();
      Utilities::String::TrimString(type);
      sfield->SetType(type);
      
      string name = results.backref(3).str();
      Utilities::String::TrimString(name);
      sfield->SetName(name);      
      
      if (!results.backref(4).str().empty())
      {
        type = sfield->GetType();
        type += "*";
        sfield->SetType(type);
      }

      sdes->AddField(sfield);
    }
    else
    {
      cout << "PARSE ERROR: [struct " << sdes->GetName() << "] member could'nt be parsed! '" << cadena << "'" << endl;
      cout << "> already remain in the string '" << test << "'" << endl;
      return false;
    }
  }
  else
  {
    cout << "PARSE ERROR: [struct " << sdes->GetName() << "] member could'nt be parsed! '" << cadena << "'" << endl;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
