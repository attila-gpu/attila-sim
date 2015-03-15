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

#include "StringToFuncDescription.h"
#include <cstring>
#include <ostream>
#include <iostream>

using namespace std;

#define DEFAULT_GROWTH 10

StringToFuncDescription::StringToFuncDescription( int initialSize ) : 
    nFuncs(0), 
    growth(DEFAULT_GROWTH)   
{
    if ( initialSize <= 0 )
        initialSize = DEFAULT_GROWTH;

    funcs = new FuncDescription*[(capacity=initialSize)];
}

bool StringToFuncDescription::resize( int inc )
{
    int tempCapacity = capacity + inc;
        
    FuncDescription** fTemp = new FuncDescription*[tempCapacity];
    
    for ( int i = 0; i < tempCapacity && i < capacity; i++ )
        fTemp[i] = funcs[i];

    if ( tempCapacity < nFuncs )
        nFuncs = tempCapacity - 1; // maybe some items erased
    
    capacity = tempCapacity;

    delete[] funcs;

    funcs = fTemp;

    return true;
}

// returns where name must be placed ( if it is not in the map ) or
// the place where it is placed
int StringToFuncDescription::findPos( const char* name ) const
{
    // ineficient
    int i;
    for ( i = 0; i < nFuncs && strcmp(name,funcs[i]->getName()) > 0; i++ ) ;    
    return i;

    // use binary search in the future
}


FuncDescription* StringToFuncDescription::find( const char* name ) const
{
    int i = findPos( name );

    if ( i == nFuncs )
        return NULL;    

    if ( strcmp(funcs[i]->getName(),name) == 0 )
        return funcs[i];
    
    return NULL;    
}

FuncDescription* StringToFuncDescription::find( int pos ) const
{
    if ( 0 <= pos && pos < nFuncs )
        return funcs[pos];
    return NULL;
}

bool StringToFuncDescription::insert( FuncDescription* fd )
{    
    if ( nFuncs == capacity )
    {
        cout << "Growing!" << endl;
        resize(growth);
    }
        
    int pos;
    
    pos = findPos( fd->getName() );

    if ( pos == nFuncs )
    {
        // add at the end
        funcs[nFuncs++] = fd;
        return true;
    }

    if ( strcmp(fd->getName(),funcs[pos]->getName()) == 0 )
    {
        funcs[pos] = fd;
        return true; // already in the map - replaced!
    }
    
    // displace data one position
    int i;
    for ( i = nFuncs-1; i >= pos; i-- )
        funcs[i+1] = funcs[i];

    funcs[pos] = fd;

    nFuncs++;
    
    return true;
}


bool StringToFuncDescription::remove( const char* name )
{    
    int pos = findPos(name);
    
    if ( pos == nFuncs )
        return false;
    
    return remove(pos);
}

bool StringToFuncDescription::remove( int pos )
{
    if ( 0 <= pos && pos < nFuncs )
    {
        for ( ; pos < nFuncs-1; pos++ )
            funcs[pos] = funcs[pos+1];
        nFuncs--;
        return true;
    }

    return false;
}

int StringToFuncDescription::count() const
{
    return nFuncs;
}

void StringToFuncDescription::dump() const
{
    for ( int i = 0; i < nFuncs; i++ )
    {
        cout << funcs[i]->toString() << endl;
    }
}


bool StringToFuncDescription::setGrowth( int growth )
{
    if ( growth < 1 )
        return false;
    this->growth = growth;
    return true;
}

int StringToFuncDescription::getGrowth() const
{
    return growth;
}
