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

#ifndef CONFIGMANAGER_H
    #define CONFIGMANAGER_H
#include <string>
#include <map>

class ConfigManager
{
public:

    bool loadConfigFile(std::string path, std::string separator = "=");
    bool saveConfigFile(std::string path, std::string separator = "=");
    std::string getOption(std::string option);
    std::pair<std::string, std::string> getOption(int pos);
    void setOption(std::string option, std::string value);

    std::map<std::string,std::string> getOptions() const { return options; }
    int countOptions() const;


private:

    std::map<std::string, std::string> options;
    std::string separator;
};

#endif // CONFIGMANAGER_H
