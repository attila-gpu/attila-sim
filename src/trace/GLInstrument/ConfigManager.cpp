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

#include "ConfigManager.h"
#include <fstream>

using namespace std;

bool ConfigManager::loadConfigFile(std::string path, std::string separator)
{
    if ( separator == "" )
        separator = "=";
    this->separator = separator;

    ifstream f(path.c_str());
    if ( !f )
        return false;
    
    char buffer[4096];

    while ( !f.eof() )
    {
        f.getline(buffer, sizeof(buffer));
        string tmp(buffer);
        int pos = tmp.find(separator);
        if ( pos != string::npos )
        {
            string option = tmp.substr(0, pos);
            string value = tmp.substr(pos+separator.length());
            options.insert(make_pair(option, value));
        }
        // else (skip line)
    }
    f.close();
    return true;
}

std::string ConfigManager::getOption(std::string option)
{
    map<string,string>::iterator it = options.find(option);
    if ( it != options.end() )
        return it->second;
    return string("");
}

int ConfigManager::countOptions() const
{
    return options.size();
}
