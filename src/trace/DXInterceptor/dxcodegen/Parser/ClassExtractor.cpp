////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utilities/Debug.h"
#include "Utilities/String.h"
#include "Config/ParserConfiguration.h"
#include "Items/ClassDescription.h"
#include "Parser/DXHParser.h"
#include "Parser/ClassExtractor.h"

using namespace std;
using namespace regex;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;
using namespace dxcodegen::Parser;

////////////////////////////////////////////////////////////////////////////////

const rpattern patro_class("(class|interface) \\s+ (\\w+) \\s* { \\s* (.*?) \\s* } \\s* ;", EXTENDED | SINGLELINE);
const rpattern patro_class_methods("\\s* (.*?) \\s* ; \\s*", "", EXTENDED | SINGLELINE | GLOBAL | ALLBACKREFS);
const rpattern patro_class_method("(\\w+) \\s+ (\\w+) \\s* \\( \\s* (.*?) \\s* \\)", "", EXTENDED | GLOBAL | ALLBACKREFS);
const rpattern patro_class_method_clean("\\s+", " ", GLOBAL | NOBACKREFS);
const rpattern patro_class_params1("\\s* , \\s*", EXTENDED);
const rpattern patro_class_params2("^(.*?)? \\s* (\\w+)$", EXTENDED);

////////////////////////////////////////////////////////////////////////////////

ClassExtractor::ClassExtractor(ParserConfiguration& config, string& cadena) :
m_config(config),
IExtractor(cadena)
{
}

////////////////////////////////////////////////////////////////////////////////

ClassExtractor::~ClassExtractor()
{
}

////////////////////////////////////////////////////////////////////////////////

const vector<ClassDescriptionPtr>& ClassExtractor::GetClasses()
{
  return m_lstClasses;
}

////////////////////////////////////////////////////////////////////////////////

ClassExtractor::MatchResults ClassExtractor::Match(string& cadena)
{
  MatchResults resultat;
  
  match_results results;
  match_results::backref_type br = patro_class.match(cadena, results);
  if (br.matched)
  {
    resultat.matched = true;
    resultat.text = results.backref(0).str();
    cadena.erase(results.rstart(0), results.rlength(0));
  }
  
  return resultat;
}

////////////////////////////////////////////////////////////////////////////////

void ClassExtractor::Parse(const ClassExtractor::MatchResults& resultat)
{
  ParseClass(resultat);
}

////////////////////////////////////////////////////////////////////////////////

void ClassExtractor::ParseClass(const ClassExtractor::MatchResults& resultat)
{
  match_results results;
  patro_class.match(resultat.text, results);
  
  // results.backref(1) was 'class' or 'interface'
  string name = results.backref(2).str();
  string subcadena = results.backref(3).str();

  if (m_config.IsClassParseCandidate(name))
  {
    if (m_verbose)
    {
      cout << "Parsing class " << name << endl;
    }
    ParseClassMethods(name, subcadena);
  }
  else
  {
    if (m_verbose)
    {
      cout << "Parsing class " << name << " -> IGNORED!" << endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ClassExtractor::ParseClassMethods(const string& name, const string& cadena)
{	
  match_results results;
  match_results::backref_type br = patro_class_methods.match(cadena, results);
  if (br.matched)
  {
    // Eliminem de la cadena (amb tots els membres de la classe) els elements
    // trobats. Si després de fer això encara queda algún caràcter en la cadena
    // significa que no l'hem parsejada totalment i per tant hi ha errors.
    // [		
    string tmpCadena(cadena);
    subst_results results_test;
    patro_class_methods.substitute(tmpCadena, results_test);
    Utilities::String::TrimString(tmpCadena);
    // ]

    if (tmpCadena.length() == 0)
    {
      ClassDescriptionPtr cdes = new ClassDescription();
      cdes->SetName(name);

      bool problem = false;
      for (size_t i=0; i < results.cbackrefs() && !problem; i+=2)
      {
        string member(results.backref(i+1).str());
        problem = !ParseClassMethod(cdes, member);
      }

      if (!problem)
      {
        m_lstClasses.push_back(cdes);
      }
    }
    else
    {
      cout << "PARSE ERROR: [class " << name << "] not all class methods processed!" << endl;
      cout << "> already remain in the string '" << tmpCadena << "'" << endl;
    }
  }
  else
  {
    cout << "PARSE ERROR: [class " << name << "] no methods found!" << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool ClassExtractor::ParseClassMethod(ClassDescriptionPtr cdes, string& cadena)
{
  // Netejem la cadena de possibles salts de línia i espais en blanc [
  subst_results results_clean;
  patro_class_method_clean.substitute(cadena, results_clean);
  Utilities::String::TrimString(cadena);
  // ]

  match_results results;
  match_results::backref_type br = patro_class_method.match(cadena, results);
  if (br.matched)
  {
    // Eliminem de la cadena (amb un membre de la classe) els elements trobats
    // Si després de fer això encara queda algún caràcter en la cadena significa
    // que no l'hem parsejada totalment i per tant hi ha errors.
    // [		
    string tmpCadena(cadena);
    subst_results results_test;
    patro_class_method.substitute(tmpCadena, results_test);
    Utilities::String::TrimString(tmpCadena);
    // ]

    if (tmpCadena.length() == 0)
    {
      MethodDescriptionPtr cmethod = new MethodDescription();
      cmethod->SetType(results.backref(1).str());
      cmethod->SetName(results.backref(2).str());

      string params(results.backref(3).str());
      if (ParseClassMethodParams(cdes, cmethod, params))
      {
        cdes->AddMethod(cmethod);
      }
    }
    else
    {
      cout << "PARSE ERROR: [class " << cdes->GetName() << "] method could'nt be parsed! '" << cadena << "'" << endl;
      cout << "> already remain in the string '" << tmpCadena << "'" << endl;
      return false;
    }
  }
  else
  {
    // Ignorem els membres d'una classe que no disposen de parentesis, el que
    // significa que es tracta d'atributs i no volem processar-los.
    if (cadena.find('(') != string::npos)
    {
      cout << "PARSE ERROR: [class " << cdes->GetName() << "] method could'nt be parsed! '" << cadena << "'" << endl;
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ClassExtractor::ParseClassMethodParams(ClassDescriptionPtr cdes, MethodDescriptionPtr cmethod, string& cadena)
{
  split_results results1;
  if (patro_class_params1.split(cadena, results1))
  {
    split_results::iterator it;
    for (it=results1.begin(); it != results1.end(); it++)
    {
      match_results results2;
      match_results::backref_type br = patro_class_params2.match(*it, results2);
      if (br.matched)
      {
        MethodDescriptionParamPtr cparam = new MethodDescriptionParam();
        cparam->SetName(results2.backref(2).str());
        cparam->SetType(results2.backref(1).str());
        cmethod->AddParam(cparam);
      }
      else
      {
        cout << "PARSE ERROR: [method " << cdes->GetName() << "::" << cmethod->GetName() << "] param could'nt be parsed! '" << *it << "'" << endl;
        return false;
      }
    }
  }
  else
  {
    cout << "PARSE ERROR: [method " << cdes->GetName() << "::" << cmethod->GetName() << "] found incorrect params! '" << cadena << "'" << endl;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
