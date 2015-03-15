////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXCodeGenException.h"
#include "Items/CppMacro.h"
#include "Parser/MacroExpander.h"

using namespace std;
using namespace regex;
using namespace dxcodegen;
using namespace dxcodegen::Items;
using namespace dxcodegen::Parser;

////////////////////////////////////////////////////////////////////////////////

MacroExpander::MacroExpander(vector<CppMacroPtr>& macros, bool verbose) :
m_lstMacros(macros),
m_verbose(verbose)
{
}

////////////////////////////////////////////////////////////////////////////////

MacroExpander::~MacroExpander()
{
}

////////////////////////////////////////////////////////////////////////////////

void MacroExpander::Expand(string& cadena)
{
  vector<CppMacroPtr>::iterator it;
  for (it=m_lstMacros.begin(); it != m_lstMacros.end(); it++)
  {
    MatchMacro(*it, cadena);
  }
}

////////////////////////////////////////////////////////////////////////////////

void MacroExpander::MatchMacro(CppMacroPtr macro, string& cadena)
{
  static const rpattern patro_macro1("\\s* (\\w+) ( \\( \\s* (\\w+) \\s* (, \\s* (\\w+) \\s*)? (, \\s* (\\w+) \\s*)? (, \\s* (\\w+) \\s*)? (, \\s* (\\w+) \\s*)? (, \\s* (\\w+) \\s*)? (, \\s* (\\w+) \\s*)? (, \\s* (\\w+) \\s*)? (, \\s* (\\w+) \\s*)? (, \\s* (\\w+) \\s*)? (, \\s* (\\w+) \\s*)? (, \\s* (\\w+) \\s*)? \\) )?", EXTENDED);
  match_results results1;
  match_results::backref_type br1 = patro_macro1.match(macro->GetLeft(), results1);
  if (br1.matched)
  {
    string macroName;
    vector<string> macroParams;
    
    // Ens guardem el nom de la macro i els seus possibles parametres [
    macroName = results1.backref(1).str();
    if (!results1.backref(2).str().empty())
    {
      for (int i=3; i < 26; i+=2)
      {
        if (!results1.backref(i).str().empty())
        {
          macroParams.push_back(results1.backref(i).str());
        }
      }
    }
    // ]
    
    // Si no detectem cap paràmetre en l'esquerra de la macro pero si veiem que
    // aquesta disposa de parentesis, llavors significa que hem detectat de
    // manera incorrecta els paràmetres de la macro i que no l'aplicarem
    // correctament. [
    bool macroApplied = false;
    if (macro->GetLeft().find('(') != string::npos)
    {
      macroApplied = (macroParams.size() != 0);
    }
    else
    {
      macroApplied = (macroParams.size() == 0);
    }
    // ]

    if (macroApplied)
    {
      // Contruim dinàmicament una expressió regular per buscar en la cadena
      // d'entrada si hi apareix la macro. [
      string macroPatro1 = macroName;
      if (macroParams.size() > 0)
      {
        macroPatro1 += " \\( ";
        for (size_t i=0; i < macroParams.size()-1; i++)
        {
          macroPatro1 += "\\s* ([\\w-\"']+) \\s* , ";
        }
        macroPatro1 += "\\s* ([\\w-\"']+) \\s* ";
        macroPatro1 += "\\)";
      }
      // ]

      const rpattern patro_macro2(macroPatro1 , EXTENDED | NOBACKREFS);
      match_results results2;
      match_results::backref_type br2 = patro_macro2.match(cadena, results2);
      macroApplied = br2.matched;
      if (macroApplied)
      {
        // Construim la cadena de substitució que s'aplicarà en cada aparició de
        // la cadena final. [
        string macroPatro2 = macro->GetRight();
        for (size_t i=0; i < macroParams.size(); i++)
        {
          char tmpString[32];
          sprintf(tmpString, "\\$%u", i+1);
          string paramName = tmpString;
          const rpattern patro_macro3(macroParams[i], paramName, GLOBAL | NOBACKREFS);
          subst_results results3;
          patro_macro3.substitute(macroPatro2, results3);
        }
        // ]

        // Apliquem en la cadena d'entrada la macro actual en totes les seves
        // aparicions. [
        macroApplied = false;
        const rpattern patro_macro4(macroPatro1, macroPatro2, EXTENDED | GLOBAL | NOBACKREFS);
        subst_results results4;
        if (patro_macro4.substitute(cadena, results4) > 0)
        {
          macroApplied = true;
        }
        // ]
      }
    }

    if (m_verbose)
    {
      if (macroApplied)
      {
        cout << "Macro applied '" << macro->GetLeft() << "'" << endl;
      }
      else
      {
        cout << "Macro NOT applied '" << macro->GetLeft() << "'" << endl;
      }
    }
  }
  else
  {
    DXCodeGenException e("macro '" + macro->GetLeft() + "' could'nt be parsed");
    throw e;
  }
}

////////////////////////////////////////////////////////////////////////////////
