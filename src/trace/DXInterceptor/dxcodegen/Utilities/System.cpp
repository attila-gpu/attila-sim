////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utilities/System.h"

using namespace std;
using namespace dxcodegen::Utilities;

////////////////////////////////////////////////////////////////////////////////

bool System::ReadEnvironmentVariable(string varName, string& varValue)
{
  varValue = "";
  char* pTable = getenv(varName.c_str());
  if (pTable != NULL)
  {
    varValue = pTable;
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

string System::GetCurrentDateTime()
{
  string datetime;
  char buffer[256];
  struct _timeb ltime;
  struct tm* today;

  _ftime(&ltime);
  today = localtime(&(ltime.time));
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", today);		
  datetime = buffer;

  return datetime;
}

////////////////////////////////////////////////////////////////////////////////
