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

#include "ParamDescription.h" // to use PD_TYPE

#ifndef NAMEMODIFIERS_H
    #define NAMEMODIFIERS_H

class NameModifiers
{
public:
    
    NameModifiers();

    NameModifiers( const char* functionName );

    void setName( const char* functionName );

    int countParams() const;

    int isVector() const;

    ParamDescription::PD_TYPE getParamsType() const;

    bool hasModifiers() const;

    ~NameModifiers();

private:

    char* name;

    int nParams;

    bool isVectorFlag;

    ParamDescription::PD_TYPE type;

    void parseName( const char* functionName );


};

#endif // NAMEMODIFIERS_H
