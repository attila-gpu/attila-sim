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

#include "NameModifiers.h"
#include <cstring>

NameModifiers::NameModifiers() : 
nParams(-1), isVectorFlag(false), type(ParamDescription::UNKNOWN), name(NULL)
{
}

void NameModifiers::parseName( const char* functionName )
{
    int i = 0;
    int len;
    bool u = false; // unsigned
    
    name = new char[(len = (int) strlen(functionName))+1];
    strcpy(name,functionName);

    // default values
    nParams = -1;
    isVectorFlag = false;
    type = ParamDescription::UNKNOWN;

    
    // find number of parameters
    for ( i = 0; i < len && (name[i] < '0' || name[i] > '9'); i++ ) ;

    if ( i == len ) // no name modifiers
        return;
    else
    {
        nParams = name[i] - 48; // convert to value
        i++;
        if ( i == len )
            return ;
        
        if ( name[i] == 'u' )
        {
            i++;
            u = true;
        }

        if ( i == len )
            return ;
            
        switch ( name[i] )
        {
            case 'b':
                type = ( u ? ParamDescription::UBYTE : ParamDescription::BYTE );
                break;
            case 's':
                type = ( u ? ParamDescription::USHORT : ParamDescription::SHORT );
                break;
            case 'i':
                type = ( u ? ParamDescription::UINT : ParamDescription::INT );
                break;
            case 'f':
                type = ParamDescription::FLOAT;
                break;
            case 'd':
                type = ParamDescription::DOUBLE;
                break;
        }
        
        i++;

        if ( i < len && name[i] == 'v' ) 
            isVectorFlag = true;

    }
}

NameModifiers::NameModifiers( const char* functionName )
{
    parseName(functionName);
}

void NameModifiers::setName( const char* functionName )
{
    if ( !name )
        delete[] name;
    parseName(functionName);
}


int NameModifiers::countParams() const
{
    return nParams;
}

int NameModifiers::isVector() const
{
    return isVectorFlag;
}

ParamDescription::PD_TYPE NameModifiers::getParamsType() const
{
    return type;
}

bool NameModifiers::hasModifiers() const
{
    if ( nParams == -1 && type == ParamDescription::UNKNOWN )
        return false;
    
    return true;
}


NameModifiers::~NameModifiers()
{
    delete[] name;
}

