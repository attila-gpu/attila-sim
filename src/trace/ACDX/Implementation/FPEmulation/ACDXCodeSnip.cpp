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

#include "ACDXCodeSnip.h"
#include <algorithm>
#include <iostream>
#include "support.h"
#include <typeinfo>

using namespace std;
using namespace acdlib;

ACDXCodeSnip::~ACDXCodeSnip()
{}

void ACDXCodeSnip::dump() const
{
    cout << text;
}

string ACDXCodeSnip::toString() const
{
    return text;
}

void ACDXCodeSnip::setCode(const std::string& code)
{
    text = code;
}


void ACDXCodeSnip::removeComments()
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



ACDXCodeSnip& ACDXCodeSnip::append(const ACDXCodeSnip& cn)
{
    if ( typeid(*this) != typeid(ACDXCodeSnip) ) /* Derivatives classes are read-only */
        panic("ACDXCodeSnip","append","Only ACDXCodeSnip base allows appending");

    text += cn.text;

    return *this;
}

ACDXCodeSnip& ACDXCodeSnip::append(std::string instr)
{
    if ( typeid(*this) != typeid(ACDXCodeSnip) ) /* Derivatives classes are read-only */
        panic("ACDXCodeSnip","append","Only ACDXCodeSnip base allows appending");

    text += instr;
    return *this;
}

ACDXCodeSnip& ACDXCodeSnip::clear()
{
    if ( typeid(*this) != typeid(ACDXCodeSnip) )
        panic("ACDXCodeSnip","clear","Only ACDXCodeSnip base allows clearing");

    text = "";
    return *this;
}
