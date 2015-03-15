////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items/EnumDescription.h"
#include "Items/StructDescription.h"
#include "Items/ClassDescription.h"
#include "Utilities/Debug.h"

using namespace std;
using namespace regex;
using namespace dxcodegen::Items;
using namespace dxcodegen::Utilities;

////////////////////////////////////////////////////////////////////////////////

void Debug::Print(vector<EnumDescriptionPtr>& enums)
{
  cout << "-*-----------------------------------------------------------------------------" << endl; 
  unsigned count = 0;
  vector<EnumDescriptionPtr>::iterator it1;
  for (it1=enums.begin(); it1 != enums.end(); it1++)
  {
    cout << "enum " << (*it1)->GetName() << endl;
    cout << "{" << endl;
    for (unsigned int i=0; i < (*it1)->GetMembersCount(); i++)
    {
      cout << "  " << (*it1)->GetMember(i)->GetName() << "," << endl;
    }
    cout << "};" << endl;
    count++;
  }
  cout << "total = " << count << " enums" << endl;
  cout << "-------------------------------------------------------------------------------" << endl;
}

////////////////////////////////////////////////////////////////////////////////

void Debug::Print(vector<StructDescriptionPtr>& structs)
{
  cout << "-*-----------------------------------------------------------------------------" << endl; 
  unsigned count = 0;
  vector<StructDescriptionPtr>::iterator it1;
  for (it1=structs.begin(); it1 != structs.end(); it1++)
  {
    cout << "struct " << (*it1)->GetName() << endl;
    cout << "{" << endl;
    for (unsigned int i=0; i != (*it1)->GetFieldCount(); i++)
    {
      cout << "  " << (*it1)->GetField(i)->GetType() << " " << (*it1)->GetField(i)->GetName() << ";" << endl;
    }
    cout << "};" << endl;
    count++;
  }
  cout << "total = " << count << " structs" << endl;
  cout << "-------------------------------------------------------------------------------" << endl;
}

////////////////////////////////////////////////////////////////////////////////

void Debug::Print(vector<ClassDescriptionPtr>& classes)
{
  cout << "-*-----------------------------------------------------------------------------" << endl; 
  unsigned int countClasses = 0;
  unsigned int countMethods = 0;
  vector<ClassDescriptionPtr>::iterator it1;
  for (it1=classes.begin(); it1 != classes.end(); it1++)
  {
    cout << "class " << (*it1)->GetName() << endl;
    cout << "{" << endl;
    for (unsigned int i=0; i < (*it1)->GetMethodsCount(); i++)
    {
      cout << "  " << (*it1)->GetMethod(i)->GetType() << " " << (*it1)->GetMethod(i)->GetName() << "(";
      for (unsigned int j=0; j != (*it1)->GetMethod(i)->GetParamsCount(); j++)
      {
        cout << (*it1)->GetMethod(i)->GetParam(j)->GetType() << ((*it1)->GetMethod(i)->GetParam(j)->GetType().empty() ? "" : " ") << (*it1)->GetMethod(i)->GetParam(j)->GetName();
        if (j+1 < (*it1)->GetMethod(i)->GetParamsCount())
        {
          cout << ", ";
        }
      }
      cout << ");" << endl;
      countMethods++;
    }
    cout << "};" << endl;
    countClasses++;
  }
  cout << "total = " << countClasses << " classes" << endl;
  cout << "total = " << countMethods << " methods" << endl;
  cout << "-------------------------------------------------------------------------------" << endl;
}

////////////////////////////////////////////////////////////////////////////////

void Debug::Print(match_results& results)
{
  cout << "-------------------------------------------------------------------------------" << endl;
  cout << "hem trobat " << (unsigned) results.cbackrefs() << " parentesis plens" << endl;
  for (size_t i=0; i < results.cbackrefs(); i++)
  {
    cout << "[" << (unsigned) i << "] = '" << results.backref(i) << "'" << endl;
  }
  cout << "-------------------------------------------------------------------------------" << endl;
}

////////////////////////////////////////////////////////////////////////////////
