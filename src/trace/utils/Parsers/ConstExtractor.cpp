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

#include "ConstExtractor.h"
#include "support.h"
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

int ConstExtractor::isPrefix(const char* str, const char* prefix)
{
    //const char* iStr = str;
    //const char* iPrefix = prefix;
    int pos = 0;
    while ( *prefix != '\0' && *str != '\0' && *str == *prefix )
    {
        prefix++;
        str++;
        pos++;
    }
    if ( *prefix == '\0' )
    {
        //cout << "\"" << iPrefix << "\" is prefix of \"" << iStr << "\"" << endl;
        return pos; // prefix length
    }
    return -1;
}

int ConstExtractor::maxMatch( const char* str1, const char* str2 )
{
    int pos = 0;
    while ( *str1 != '\0' && *str2 != '\0' && *str1 == *str2 )
    {
        str1++;
        str2++;
        pos++;
    }
    return pos;
}

ConstExtractor::ConstExtractor() : iConst(0)
{

}

bool ConstExtractor::matchWithFilter( const char* name )
{
    unsigned int i;

    for ( i = 0; i < filter.size(); i++ ) 
    {
        if ( strcmp( name, filter[i] ) == 0 )
            return true;
    }    
    return false;
}

int ConstExtractor::extractConstants( const char* file )
{
    ifstream f;
    char line[1024];

    f.open( file, fstream::in /*| fstream::nocreate*/);

    if ( !f.is_open() )
        return -1;

    const ConstDescription* cd;

    int i = 0;

    while ( !f.eof() )
    {
        f.getline(line, sizeof(line));

        cd = ConstDescription::parseConstant(line);

        if ( cd != NULL ) 
        {
            i++;
            if ( !matchWithFilter(cd->getString()) )
                cdv[iConst++] = cd;
            else // skipped by filter
                delete cd;
        }
    }
    return i;
}

int ConstExtractor::count() const
{
    return iConst;
}

const ConstDescription* ConstExtractor::getConstant( int pos ) const
{
    return ( 0 <= pos && pos < iConst ? cdv[pos] : NULL );
}

const ConstDescription* ConstExtractor::getConstant( const char* name ) const
{
    int i;
    for ( i = 0; i < iConst; i++ )
    {
        if ( strcmp(cdv[i]->getString(), name) == 0 )
            return cdv[i];
    }
    return NULL;
}

std::vector<int>* ConstExtractor::getConstantPosition( int value, int maxPositions ) const
{
    std::vector<int>* v = new std::vector<int>;
    int i;
    for ( i = 0; i < iConst && maxPositions != 0; i++ )
    {
        if ( cdv[i]->getValue() == value )
        {
            v->push_back(i);
            maxPositions--;
        }
    }
    if ( v->size() > 0 )
        return v;
    delete v;
    return NULL;
}


int ConstExtractor::getConstantPosition( int value ) const
{
    int i;
    for ( i = 0; i < iConst; i++ )
    {
        if ( cdv[i]->getValue() == value )
            return i;
    }
    return -1;
}


int ConstExtractor::getConstantPosition( const char* name ) const
{
    int i;
    for ( i = 0; i < iConst; i++ )
    {
        if ( strcmp(cdv[i]->getString(),name) == 0 )
            return i;
    }
    return -1;
}

void ConstExtractor::dump()
{
    int i;

    for ( i = 0; i < iConst; i++ )
        cout << cdv[i]->toString() << endl;
}

void ConstExtractor::addFilter( const char* aFilter )
{
    char* f = new char[strlen(aFilter)+1];
    strcpy(f, aFilter);
    filter.push_back(f);
}

ConstExtractor::~ConstExtractor()
{
    unsigned int i;
    for ( i = 0; i < filter.size(); i++ )
        delete[] filter[i];
    // remove internal elements before vector removing
}
