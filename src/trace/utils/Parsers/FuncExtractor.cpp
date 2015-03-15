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

#include "FuncExtractor.h"
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

typedef std::vector<const char*> FilterVector;
typedef std::vector<const char*>::iterator FilterIterator;

FuncExtractor::FuncExtractor() : nameToFunc(5000)
{
    // empty
}

bool FuncExtractor::matchWithFilter( const char* name )
{
    unsigned int i;

    for ( i = 0; i < filter.size(); i++ ) 
    {
        if ( filter[i][0] == '*' )
        {
            if ( strstr( name, &filter[i][1] ) != NULL ) 
                return true;
        }
        else 
        {
            if ( strcmp( name, filter[i] ) == 0 )
                return true;
        }
    }    
    return false;
}


bool FuncExtractor::hasReservedWords( const char* stream )
{
    if ( strstr(stream,"typedef") != NULL )
        return true;
    if ( strstr(stream,"#define") != NULL )
        return true;
    return false;
}

int FuncExtractor::extractFunctions( const char* file )
{
    char line[1024];
    int i, iFuncs = 0;    
    
    ifstream f;    

    f.open( file, ios::in /* | ios::nocreate*/ );    

    if ( !f.is_open() )
        return -1;

    while ( !f.eof() )
    {
        /*
         * A function may be defined in several lines
         * Merge all function definition in one line after perform parsing
         */
        f.getline( line, sizeof( line ) );
        
        if ( hasReservedWords(line) ) {
            //cout << "SKIPED: \"" << line << "\"" << endl;
            continue; // skip line
        }
        
        if ( strncmp(line, "GLAPI", 5) == 0 || strncmp(line, "WGLAPI", 6) == 0 ) {
            /* It is a gl.h, glext.h or wgl_mesa.h definition.
               A function can be defined in several lines */
            /* check for ";" */
            while ( true ) 
            {
                for ( i = 0; line[i] != 0 && line[i] != ';' ; i++ ) ;
                /*  no ';' found ( function is defined in  more than 1 line ) */
                if ( line[i] == 0 ) 
                    f.getline( &line[i], sizeof(line) - i );
                else
                    break;
            }
        }
                
        /* Try to parse the function */        
        FuncDescription* funcObj = FuncDescription::parseFunc( line, sizeof(line) );
        
        if ( funcObj != NULL ) {

            if ( !matchWithFilter(funcObj->getName()) )
            {
                // filter already defined functions
                if ( nameToFunc.find(funcObj->getName()) == NULL ) {
                    iFuncs++;
                    nameToFunc.insert(funcObj);
                }
                //else
                //    cout << "already present: " << funcObj->getName() << endl;
            }
            else 
            {
                //cout << funcObj->getName() << " FILTERED!" << endl;
                delete funcObj;
            }
        }
        else 
        {   
            //cout << "Not well parsed: " << line << endl;
            /* Check if some function could not be parsed */
            if ( strncmp( line, "GLAPI", 5 ) == 0 || strncmp(line,"WGLAPI",6) == 0 )
                cout << "(It should not happen) Discarded!: " << line << endl;
        }
    }
  
    /* Number of functions parsed */
    return iFuncs;
}

int FuncExtractor::count() const
{
    return nameToFunc.count();
}


void FuncExtractor::dump() const
{       
    int nFuncs = nameToFunc.count();

    for ( int i = 0; i < nFuncs; i++ )
        cout << nameToFunc.find(i)->toString() << endl;
}

const FuncDescription* FuncExtractor::getFunction( const char* name ) const
{
    return nameToFunc.find(name);
}


const FuncDescription* FuncExtractor::getFunction( int pos ) const
{
    return nameToFunc.find(pos);
}


void FuncExtractor::addFilter( const char* aFilter )
{
    char* ft = new char[strlen(aFilter)+1];
    strcpy(ft, aFilter);
    filter.push_back(ft);
}

int FuncExtractor::applyFilter()
{
    
    int i, deletes = 0;
    FuncDescription*  fd;
    for ( i = nameToFunc.count()-1; i >= 0; i-- )
    {
        fd = nameToFunc.find(i);
        if ( matchWithFilter(fd->getName()) ) 
        {
            nameToFunc.remove(i);
            deletes++;
        }
    }
    return deletes;
}


void FuncExtractor::removeFilter( const char* aFilter )
{
    
    FilterIterator it = filter.begin();
    FilterIterator last = filter.end();
    
    for ( it; it != last; it++ )
    {
        if ( strcmp(*it,aFilter) == 0 )        
            filter.erase(it);
    }
}


void FuncExtractor::clearFilters()
{
    filter.clear();
}
