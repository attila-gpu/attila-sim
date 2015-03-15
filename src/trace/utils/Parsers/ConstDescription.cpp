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

#include "ConstDescription.h"

#include "StringTokenizer.h"

#include <cstring>
#include <cstdio>
#include <ostream>

using namespace std;

#ifndef NULL
    #define NULL 0
#endif

#define DELIMITERS " \t"

#define EQ(a,b) (strcmp(a,b) == 0)
#define EQN(a,b,n) (strncmp(a,b,n) == 0)

char ConstDescription::toStringBuffer[256];

ConstDescription::ConstDescription() {}

const ConstDescription* ConstDescription::parseConstant( const char* constantDef )
{
    StringTokenizer st(constantDef);

    st.setDefaultDelimiters(DELIMITERS);

    char token[256];

    if ( st.getToken(token, sizeof(token)) == NULL )
        return NULL;

    if ( EQ(token,"#") )
    {
        st.getToken(token, sizeof(token));
        if ( !EQ(token,"define") )
            return NULL;
    }
    else if ( EQ(token,"#define") )
        ;
    else
        return NULL;

    if ( st.getToken(token, sizeof(token)) == NULL )
        return NULL; /* Valid name not found */
   
    if ( !EQN(token,"GL",2) && !EQN(token,"WGL",3) )
        return NULL;

    ConstDescription* cd = new ConstDescription;

    cd->name = new char[strlen(token)+1];

    strcpy(cd->name, token); /* Copy name */

    if ( st.getToken(token, sizeof(token)) == NULL )
    {
        delete cd;
        return NULL;
    }

    cd->value = parseHex(token);

    if ( cd->value < 0 )
    {
        delete cd;
        return NULL;
    }

    return cd;
}


ConstDescription::~ConstDescription()
{
    delete[] name;
}

int ConstDescription::parseHex( const char* hexStr )
{    
    const char* current = hexStr;
    int value = 0;
    bool ok = false;
    
    while ( (*current == ' ' || *current == '\t') && *current != '\0' )
        current++;            

    if ( *current == '\0' )
        return -1;
    else if ( *current != '0' )
        return -1;
    
    current++; /* Skip '0' */

    if ( *current == '\0' )
        return -1;
    else if ( *current != 'x' && *current != 'X' )
        return -1;

    current++; /* Skip 'x' or 'X' */

    while ( *current != '\0' &&
            (('0' <= *current && *current <= '9') ||
             ('a' <= *current && *current <= 'f') ||
             ('A' <= *current && *current <= 'F')) )
    {
        ok = true; // at least one valid char
        if ( '0' <= *current && *current <= '9' )
            value = (value << 4) + (*current - '0');
        else if ('a' <= *current && *current <= 'f')
            value = (value << 4) + (10 + *current - 'a');
        else
            value = (value << 4) + (10 + *current - 'A');
        
        current++;
    }
    
    return ( ok ? value :  - 1);
}

int ConstDescription::getValue() const
{
    return value;
}

const char* ConstDescription::getString() const
{
    return name;
}


const char* ConstDescription::toString() const
{
    sprintf(toStringBuffer,"%s = 0x%x",name, value);
    return toStringBuffer;
}
