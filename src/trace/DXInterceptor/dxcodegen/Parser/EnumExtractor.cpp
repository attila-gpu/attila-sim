////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utilities/String.h"
#include "Config/ParserConfiguration.h"
#include "Items/EnumDescription.h"
#include "Parser/DXHParser.h"
#include "Parser/EnumExtractor.h"

using namespace std;
using namespace regex;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;
using namespace dxcodegen::Parser;

#define TYPEDEF_ENUM 1
#define ENUM 2

////////////////////////////////////////////////////////////////////////////////

const rpattern patro_typedef_enum("typedef \\s+ enum \\s+ (\\w+)? \\s* { \\s* (.*?) \\s* } \\s* ([^;]+) ;", EXTENDED | SINGLELINE);
const rpattern patro_typedef_enum_names_split(",");
const rpattern patro_typedef_enum_names_clean("\\s+", "", GLOBAL | NOBACKREFS);
const rpattern patro_enum("enum \\s+ (\\w+) \\s* { \\s* (.*?) \\s* } \\s* [^;]* ;", EXTENDED | SINGLELINE);
const rpattern patro_enum_members("^(\\s* \\w+ \\s* = \\s* \\w+ \\( .*? \\) \\s* | \\s* \\w+ \\s* = [^,]+ | \\s* \\w+ \\s*) ,?",     EXTENDED | MULTILINE | SINGLELINE | GLOBAL | ALLBACKREFS);
const rpattern patro_enum_members_test("^(\\s* \\w+ \\s* = \\s* \\w+ \\( .*? \\) \\s* | \\s* \\w+ \\s* = [^,]+ | \\s* \\w+ \\s*) ,?", "", EXTENDED | MULTILINE | SINGLELINE | GLOBAL | NOBACKREFS);
const rpattern patro_enum_member("^(\\w+) ((\\s* = .*?)?)$", EXTENDED | GLOBAL | ALLBACKREFS);
const rpattern patro_enum_member_test("^(\\w+) ((\\s* = .*?)?)$", "", EXTENDED | GLOBAL | NOBACKREFS);
const rpattern patro_enum_member_clean("\\s+", " ", GLOBAL | NOBACKREFS);

////////////////////////////////////////////////////////////////////////////////

EnumExtractor::EnumExtractor(ParserConfiguration& config, string& cadena) :
m_config(config),
IExtractor(cadena)
{
}

////////////////////////////////////////////////////////////////////////////////

EnumExtractor::~EnumExtractor()
{
}

////////////////////////////////////////////////////////////////////////////////

const vector<EnumDescriptionPtr>& EnumExtractor::GetEnums()
{
  return m_lstEnums;
}

////////////////////////////////////////////////////////////////////////////////

EnumExtractor::MatchResults EnumExtractor::Match(string& cadena)
{
  MatchResults resultat;
  
  resultat = MatchTypedefEnum(cadena);
  if (resultat.matched)
  {
    return resultat;
  }

  resultat = MatchEnum(cadena);
  if (resultat.matched)
  {
    return resultat;
  }

  return resultat;
}

////////////////////////////////////////////////////////////////////////////////

EnumExtractor::MatchResults EnumExtractor::MatchTypedefEnum(string& cadena)
{
  MatchResults resultat;

  match_results results;
  match_results::backref_type br = patro_typedef_enum.match(cadena, results);
  if (br.matched)
  {
    resultat.matched = true;
    resultat.type = TYPEDEF_ENUM;
    resultat.text = results.backref(0).str();
    cadena.erase(results.rstart(0), results.rlength(0));
  }

  return resultat;
}

////////////////////////////////////////////////////////////////////////////////

EnumExtractor::MatchResults EnumExtractor::MatchEnum(string& cadena)
{
  MatchResults resultat;

  match_results results;
  match_results::backref_type br = patro_enum.match(cadena, results);
  if (br.matched)
  {
    resultat.matched = true;
    resultat.type = ENUM;
    resultat.text = results.backref(0).str();
    cadena.erase(results.rstart(0), results.rlength(0));
  }

  return resultat;
}

////////////////////////////////////////////////////////////////////////////////

void EnumExtractor::Parse(const EnumExtractor::MatchResults& resultat)
{
  switch (resultat.type)
  {
  case TYPEDEF_ENUM:
    ParseTypedefEnum(resultat);
    break;

  case ENUM:
    ParseEnum(resultat);
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void EnumExtractor::ParseTypedefEnum(const EnumExtractor::MatchResults& resultat)
{
  match_results results;
  patro_typedef_enum.match(resultat.text, results);

  string name = results.backref(3).str();
  string subcadena = results.backref(2).str();

  // Subdividim la llista de noms del typedef per tractar-los cadascun per
  // separat [
  split_results results_split;
  size_t num = patro_typedef_enum_names_split.split(name, results_split);
  // ]

  split_results::string_vector vec_names = results_split.strings();
  for (size_t i=0; i < vec_names.size(); i++)
  {
    name = vec_names[i];

    // Netejem el nom de possibles espais en blanc [
    subst_results results_clean;
    patro_typedef_enum_names_clean.substitute(name, results_clean);
    // ]

    if (m_config.IsEnumParseCandidate(name))
    {
      if (m_verbose)
      {
        cout << "Parsing typedef enum " << name << endl;
      }
      ParseEnumMembers(name, subcadena);
    }
    else
    {
      if (m_verbose)
      {
        cout << "Parsing typedef enum " << name << " -> IGNORED!" << endl;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void EnumExtractor::ParseEnum(const EnumExtractor::MatchResults& resultat)
{
  match_results results;
  patro_enum.match(resultat.text, results);

  string name = results.backref(1).str();
  string subcadena = results.backref(2).str();

  if (m_config.IsEnumParseCandidate(name))
  {
    if (m_verbose)
    {
      cout << "Parsing enum " << name << endl;
    }
    ParseEnumMembers(name, subcadena);
  }
  else
  {
    if (m_verbose)
    {
      cout << "Parsing enum " << name << " -> IGNORED!" << endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void EnumExtractor::ParseEnumMembers(const string& name, const string& cadena)
{	
  match_results results;
  match_results::backref_type br = patro_enum_members.match(cadena, results);
  if (br.matched)
  {
    // Eliminem de la cadena (amb tots els membres del enum) els elements
    // trobats. Si després de fer això encara queda algún caràcter en la cadena
    // significa que no l'hem parsejada totalment i per tant hi ha errors.
    // [		
    string test(cadena);
    subst_results results_test;
    patro_enum_members_test.substitute(test, results_test);
    Utilities::String::TrimString(test);
    // ]

    if (test.length() == 0)
    {
      EnumDescriptionPtr edes = new EnumDescription();
      edes->SetName(name);			

      bool problem = false;
      for (size_t i=0; i < results.cbackrefs() && !problem; i+=2)
      {
        string member(results.backref(i+1).str());
        problem = !ParseEnumMember(edes, member);
      }

      if (!problem)
      {
        m_lstEnums.push_back(edes);
      }
    }
    else
    {
      cout << "PARSE ERROR: [enum " << name << "] not all enum members processed!" << endl;
      cout << "> already remain in the string '" << test << "'" << endl;
    }
  }
  else
  {
    cout << "PARSE ERROR: [enum " << name << "] no members found!" << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool EnumExtractor::ParseEnumMember(EnumDescriptionPtr edes, string& cadena)
{
  // Netejem la cadena de possibles salts de línia i espais en blanc [
  subst_results results_clean;
  patro_enum_member_clean.substitute(cadena, results_clean);
  Utilities::String::TrimString(cadena);
  // ]

  match_results results;
  match_results::backref_type br = patro_enum_member.match(cadena, results);
  if (br.matched)
  {
    // Eliminem de la cadena (amb un membre del enum) els elements trobats
    // Si després de fer això encara queda algún caràcter en la cadena significa
    // que no l'hem parsejada totalment i per tant hi ha errors.
    // [		
    string test(cadena);
    subst_results results_test;
    patro_enum_member_test.substitute(test, results_test);
    Utilities::String::TrimString(test);
    // ]

    if (test.length() == 0)
    {
      EnumDescriptionMemberPtr member = new EnumDescriptionMember();
      string name = results.backref(1).str();
      Utilities::String::TrimString(name);
      member->SetName(name);
      
      edes->AddMember(member);
    }
    else
    {
      cout << "PARSE ERROR: [enum " << edes->GetName() << "] member could'nt be parsed! '" << cadena << "'" << endl;
      cout << "> already remain in the string '" << test << "'" << endl;
      return false;
    }
  }
  else
  {
    cout << "PARSE ERROR: [enum " << edes->GetName() << "] member could'nt be parsed! '" << cadena << "'" << endl;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
