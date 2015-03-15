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

#include "GLResolver.h"
#include "support.h"

#include <cstring>
#include <iostream>

using namespace std;

const char* GLResolver::fnames[] = 
{
    // all function names
    #include "GLFunctionNames.gen"
};

GLResolver::NameToValue GLResolver::constants[] =
{
    #include "GLConstantNames.gen" // include all {constant, value} pairs
};

map<string, int> GLResolver::constantTable;

bool GLResolver::constantTableCreated = false;

void GLResolver::createConstantTable()
{
    constantTableCreated = true;

    constantTable.clear();

    for(int i = 0; i < COUNT_ARRAY(constants); i++)
    {
        string str;
        str = constants[i].name;
        constantTable[str] = constants[i].value;
    }
}

APICall GLResolver::getFunctionID( const char* name )
{
    int i = 0;
    for ( i = 0; i < COUNT_ARRAY(fnames); i++ )
    {
        if ( EQ(name,fnames[i]) )
            return (APICall)i;
    }
    return APICall_UNDECLARED;
}

vector<APICall>* GLResolver::getFunctionNameMatching( const char* match )
{
    

    int i, j;

    char m[256];
    
    int len;

    int kindOfMatch;

    /* Find out kind of matching
     * 1: has the match
     * 2: start with match
     * 3: finish with match
     */
    if ( match == NULL )
        return NULL;

    len = (int) strlen(match);
    
    if ( match[0] == '*' && match[len-1] == '*' )
    {
        strncpy(m, &match[1], len-1);
        len = len - 2;
        kindOfMatch = 1;
    }
    else if (match[0] == '*' )
    {
        strcpy(m, &match[1]);
        len--;
        kindOfMatch = 3;
    }
    else if ( match[len-1] == '*' )
    {
        strncpy(m, match, len-1);
        len--;
        kindOfMatch = 2;
    }
    else
        return NULL; /* it is not a valid pattern matching */

    vector<APICall> *v = new vector<APICall>;
    
    for ( i = 0; i < COUNT_ARRAY(fnames); i++ )
    {
        switch ( kindOfMatch )
        {
            case 1: /* has the match */
                if ( strstr(fnames[i], m) != NULL )
                    v->push_back((APICall)i);
                break;
            case 2: /* starts with match */
                if ( strncmp(fnames[i], m, len) == 0 )
                    v->push_back((APICall)i);
                break;
            case 3: /* ends with match */
                j = (int) strlen(fnames[i]);
                if ( len > j )
                    break;

                if ( strcmp(&((fnames[i])[j-len]), m) == 0 )
                    v->push_back((APICall)i);
                break;
        }
    }

    if ( v->size() == 0 )
    {
        delete v;
        return NULL;
    }

    return v;

}

const char* GLResolver::getFunctionName( APICall id )
{
    if ( 0 > (int)id || (int)id >= COUNT_ARRAY(fnames) )
        return NULL;
    return fnames[(int)id];
}

int GLResolver::countFunctions()
{
    return COUNT_ARRAY(fnames);
}

int GLResolver::countConstants()
{
    return (int)APICall_UNDECLARED;
}


int GLResolver::getConstantValue( const char* name )
{
    /*int i = 0;
    for ( i = 0; i < COUNT_ARRAY(constants); i++ )
    {
        if ( EQ(name,constants[i].name) )
            return constants[i].value;
    }*/

    if (!constantTableCreated)
        createConstantTable();

    string str = name;
    map<string,int>::iterator iter = constantTable.find(str);
    if (iter != constantTable.end())
    {
        return iter->second;
    }
    else
    {
        return -1;
    }
}

const char* GLResolver::getConstantName( int id )
{
    int i;
    for ( i = 0; i < COUNT_ARRAY(constants); i++ )
    {
        if ( id == constants[i].value )
            return constants[i].name;
    }
    return NULL;
    
}

