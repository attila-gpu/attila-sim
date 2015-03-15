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

/*****************************************************************************
Simple windows INI file style configuration.

Author: Jos� Manuel Sol�s
*****************************************************************************/

#ifndef __INI_FILE
#define __INI_FILE

class IniFile;

class IniFileSection {
friend class IniFile;
public:
    std::string getName();
    void setName(std::string);
    bool existsVariable(std:: string variable);
    std::string getValue(std::string variable);
    void assign(std::string variable, std::string value);
private:
    std::string name;
    std::map<std::string, std::string> variables;
    void save(std::ostream &stream);
};

class IniFile {
public:
    bool load(std::string filename);
    bool save(std::string filename);
    IniFileSection *getSection(std::string name);
    ~IniFile();    
private:
    bool isSectionName(std::string s);
    std::string getSectionName(std::string s);

    bool isComentary(std::string s);
    bool isAssignation(std::string s);
    std::string getVariable(std::string s);
    std::string getValue(std::string s);

    std::map<std::string, IniFileSection *> sections;
};



#endif
