////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utilities/String.h"

using namespace std;
using namespace dxcodegen::Utilities;

////////////////////////////////////////////////////////////////////////////////

void String::TrimString(std::string& cadena)
{
  string::size_type pos = cadena.find_first_not_of(" \t\r\n");
  if (pos != std::string::npos)
  {
    cadena.erase(0, pos);
  }
  else if (cadena.length() != 0)
  {
    cadena.clear();
  }

  pos = cadena.find_last_not_of(" \t\r\n");
  if (pos != std::string::npos)
  {
    cadena.erase(pos+1, std::string::npos);
  }
}

////////////////////////////////////////////////////////////////////////////////
