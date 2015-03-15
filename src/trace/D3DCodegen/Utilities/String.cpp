////////////////////////////////////////////////////////////////////////////////

#include "Utilities/String.hpp"

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
  else
    if (cadena.length() != 0)
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


void String::ToLower(std::string& s) {
    char* buf = new char[s.length()];
    s.copy(buf, s.length());
    for(unsigned int i = 0; i < s.length(); i++)
        buf[i] = toupper(buf[i]);
    string temp(buf, s.length());
    delete buf;
    s = temp;
}

////////////////////////////////////////////////////////////////////////////////

void String::ToUpper(std::string& s) {
    char* buf = new char[s.length()];
    s.copy(buf, s.length());
    for(unsigned int i = 0; i < s.length(); i++)
        buf[i] = toupper(buf[i]);
    string temp(buf, s.length());
    delete buf;
    s = temp;
}

////////////////////////////////////////////////////////////////////////////////

void String::Replace(std::string &s, std::string &find, std::string &replace) {
    size_t x1 = s.find(find);
    if(x1 != s.npos) {
        s.erase(x1, find.length());
        s.insert(x1, replace);
    }
 }
