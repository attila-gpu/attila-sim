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

#include "stdafx.h"
#include "IniFile.h"
#include "D3DConfiguration.h"

using namespace std;

D3DConfiguration &D3DConfiguration::instance() {
    static D3DConfiguration me;
    return me;
}

void D3DConfiguration::load(string filename) {
    ini.load(filename);
}

string D3DConfiguration::getValue(string section, string variable) {
    IniFileSection *pSection = ini.getSection(section);
    if(pSection == 0) return "";
    else return pSection->getValue(variable);
}

bool D3DConfiguration::existsVariable(std::string section, std::string variable)
{
    IniFileSection *pSection = ini.getSection(section);
    if(pSection == 0) return false;
    else return pSection->existsVariable(variable);
}
