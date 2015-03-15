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

#include "CodeSnip.h"
#include <algorithm>
#include <iostream>
#include "support.h"
#include <typeinfo>

using namespace std;

CodeSnip::~CodeSnip()
{}

void CodeSnip::dump() const
{
    cout << text;
}

string CodeSnip::toString() const
{
    return text;
}

void CodeSnip::setCode(const std::string& code)
{
    text = code;
}


void CodeSnip::removeComments()
{
    string::size_type pos = text.find_first_of('#');
    string::size_type pos2;
    while ( pos != string::npos )
    {
        pos2 = text.find_first_of('\n', pos);
        if ( pos2 == string::npos )
            pos2 = text.length();

        text = text.erase(pos, pos2 - pos); // remove comment
        pos = text.find_first_of('#');
    }
}



CodeSnip& CodeSnip::append(const CodeSnip& cn)
{
    if ( typeid(*this) != typeid(CodeSnip) ) /* Derivatives classes are read-only */
        panic("CodeSnip","append","Only CodeSnip base allows appending");

    text += cn.text;

    return *this;
}

CodeSnip& CodeSnip::append(std::string instr)
{
    if ( typeid(*this) != typeid(CodeSnip) ) /* Derivatives classes are read-only */
        panic("CodeSnip","append","Only CodeSnip base allows appending");

    text += instr;
    return *this;
}

CodeSnip& CodeSnip::clear()
{
    if ( typeid(*this) != typeid(CodeSnip) )
        panic("CodeSnip","clear","Only CodeSnip base allows clearing");

    text = "";
    return *this;
}
