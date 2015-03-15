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

#include <string>
#include <vector>

using namespace std;

string trim(string& s,const string& drop = " \t"){
 std::string r=s.erase(s.find_last_not_of(drop)+1);
 return r.erase(0,r.find_first_not_of(drop));
}

vector<string> explode(string s, const string& by){

    vector<string> ret;
    int iPos = s.find(by, 0);
 
    while (iPos != s.npos)
    {
        if(iPos!=0)
            ret.push_back(s.substr(0,iPos));
        s.erase(0,iPos + 1);

        iPos = s.find(by, 0);
    }

    if(s.length() > 0)
        ret.push_back(s);

    return ret;

}


vector<string> explode_by_any(string s, const string& characters){

    vector<string> ret;
    int iPos = s.find_first_of(characters, 0);
 
    while (iPos != -1)
    {
        if(iPos!=0)
            ret.push_back(s.substr(0,iPos));
        s.erase(0,iPos + 1);

        iPos = s.find_first_of(characters, 0);
    }

    if(s.length() > 0)
        ret.push_back(s);

    return ret;

}
