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

#include "SpecificExtractor.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;


bool SpecificItem::logReturn() const
{
    return logReturnFlag;
}

void SpecificItem::setLogReturn( bool value )
{
    logReturnFlag = value;
}

void SpecificItem::setOCSFlag( bool ocsFlag )
{
    ocs = ocsFlag;
}

bool SpecificItem::getOCSFlag() const
{
    return ocs;
}

int SpecificExtractor::parseValues( const char* buffer, int values[], int maxValues, bool& ret)
{
    // expected list of ints, format: "v1,v2,...,vn>" r or R are also supported (without quotes)
        
    ret = false;
    char aux[256];
    int j = 0, iValue = 0, i = 0;

    for ( ; buffer[i] != '>' && buffer[i] != '\0' && iValue < maxValues; i++ )
    {
        if ( buffer[i] != ',' )
        {
            aux[j++] = buffer[i];
        }
        else
        {
            if ( j != 0 ) // if some data read, parse it. If not, skip parsing
            {
                aux[j] = '\0';
                if ( strcmp("r",aux) == 0 || strcmp("R",aux) == 0 ) 
                {
                    ret = true;
                }
                else 
                    values[iValue++] = atoi(aux); // parse int
                j = 0; // reset index pointing to 'aux' buffer
            }
        }
    }
    if ( buffer[i] == '>' ) 
    {
        // process last
        if ( j != 0 )
        {
            aux[j] = '\0';
            if ( strcmp("r",aux) == 0 || strcmp("R",aux) == 0 )
                ret = true;
            else
                values[iValue++] = atoi(aux);
        }
        return iValue;
    }
    else
        return -1; // list cannot be parsed ( not closed or not well-formed )

}

SpecificItem::SpecificItem() : fd(NULL), howManyCalls(0), replaceFlag(false), ocs(false),
                               callAfterReturn(false), logReturnFlag(true)
{
    memset(disabledParams, 0, sizeof(disabledParams)); // all params enabled
}

void SpecificItem::setFunction( const FuncDescription* fd )
{
    this->fd = fd;
}

const FuncDescription* SpecificItem::getFunction() const
{
    return fd;
}


void SpecificItem::setParam( int position, bool value )
{
    if ( position > 0 && position <= MAX_PARAMS )
        disabledParams[position-1] = value;
}
    
bool SpecificItem::getParam( int position ) const
{
    if ( position > 0 && position <= MAX_PARAMS )
        return disabledParams[position-1];
    return false;
}


void SpecificItem::setReplaceFlag( bool replaceFlag )
{
    this->replaceFlag = replaceFlag;
}

bool SpecificItem::getReplaceFlag() const
{
    return replaceFlag;
}

void SpecificItem::dump() const
{
    bool one = false;
    int i;
    cout << "Function: " << fd->toString() << endl;
    cout << "Params not logged: {";
    for ( i = 0; i < fd->countParams(); i++ )
    {
        if ( disabledParams[i] ) 
        {
            if ( one )
                cout << ",";
            one = true;
            cout << i;
        }
    }
    cout << "}" << endl;
    cout << "(Note: Params are numbered from 1 to N, pos 0 means before first param)" << endl;
    cout << "Position where calls will be generated (after params): " << endl;
    cout << "{";
    for ( i = 0; i < howManyCalls; i++ )
    {
        cout << listOfCalls[i];
        if ( i < howManyCalls-1 )
            cout << ",";
    }
    cout << "}" << endl;
}


SpecificExtractor::SpecificExtractor() : iItem(0)
{
}

void SpecificExtractor::dump() const
{
    for ( int i = 0; i < iItem; i++ )
    {
        items[i]->dump();
    }
}

int SpecificExtractor::extract( const char* specificFileName )
{
    bool ret = false;
    char line[1024];

    ifstream f;
    
    f.open(specificFileName, /*ios::nocreate | */ios::in);

    if ( !f.is_open() )
        return -1;
    
    int i, j, itemsExtracted = 0;
    
    SpecificItem* si = NULL;
    int values[20]; // max number of values in lists
    
    while ( !f.eof() )
    {
        f.getline(line, sizeof(line));
        
        // Skip whites
        for ( i = 0; line[i] != 0 && ( line[i] == ' ' || line[i] == '\t' ); i++ ) ; // skip whites
        
        if ( line[i] != 0 )
        { // line with contents                        
            // OPTIONS
            if ( strncmp(&line[i], "//<", 3) == 0 ) 
            {                
                i += 3; // skip "//<"
                if ( !si ) // Create new SpecificItem if required
                    si = new SpecificItem;
                
                if ( strncmp(&line[i], "NL=",3) == 0 )
                { // disable parameter log TAG
                    i += 3;
                    if ( (i = parseValues(&line[i],values,sizeof(values),ret)) >= 0 )
                    {
                        for ( j = 0; j < i; j++ ) {
                            if ( values[j] > 0 )
                                si->setParam(values[j], true);
                        }
                    }
                    if ( ret )
                        si->setLogReturn(false);                    
                }
                else if ( strncmp(&line[i],"A=",2) == 0 )
                { // TAG position
                    i += 2;
                    if ( (i = parseValues(&line[i],values,sizeof(values),ret)) >= 0 )
                    {
                        for ( j = 0; j < i; j++ )
                            si->addSpecificCall(values[j]);
                    }
                    if ( ret )
                        si->addSpecificCallAfterReturn();
                }
                else if ( strncmp(&line[i],"R>",2) == 0 )
                {
                    i+=2;
                    si->setReplaceFlag(true);
                }
                else if ( strncmp(&line[i],"OCS>",4) == 0 )
                {
                    i+=4;
                    si->setOCSFlag(true);                    
                }
                // Option tag found
            }
            // NOT AN OPTION
            else {
                FuncDescription* fd = FuncDescription::parseFunc(line, sizeof(line));
                if ( fd != NULL )
                {
                    if ( si == NULL ) // function without special options
                        si = new SpecificItem;
                    si->setFunction(fd);
                    items[iItem++] = si;
                    itemsExtracted++;
                    si = NULL;
                }
            }
        }
    }    
    return itemsExtracted;
}

SpecificItem* SpecificExtractor::getSpecificFunction( const char* originalFunctionName ) const
{
    char buffer[256] ="";
    strcpy(buffer, originalFunctionName);
    strcat(buffer, SPECIFIC_SUFIX);
    int i;
    for ( i = 0; i < iItem; i++ )
    {
        if ( strcmp(items[i]->getFunction()->getName(),buffer) == 0 )
            return items[i];
    }
    return NULL;
}

int SpecificExtractor::count() const
{
    return iItem;
}

int SpecificItem::howManySpecificCalls() const
{
    return howManyCalls;
}

void SpecificItem::addSpecificCall( int pos )
{
    if ( pos < 0 )
        return ; // ignore negative values
    // keep ordered
    int i, j;
    for ( i = 0; i < howManyCalls; i++ )
    {
        if ( pos <= listOfCalls[i] )
        {
            if ( pos == listOfCalls[i] )
                return; // already specified
            for ( j = howManyCalls-1; j >= listOfCalls[i]; j-- )
                listOfCalls[j+1] = listOfCalls[j];
            listOfCalls[i] = pos;
        }
    }
    if ( i == howManyCalls )
        listOfCalls[howManyCalls] = pos;

    howManyCalls++;
}

bool SpecificItem::isSpecificCallRequired( int afterParam ) const
{
    int i;
    
    for ( i = 0; i < howManyCalls && listOfCalls[i] < afterParam; i++ ) ;

    if ( i < howManyCalls )
        return (listOfCalls[i] == afterParam);
    return false;
}


void SpecificItem::addSpecificCallAfterReturn()
{
    callAfterReturn = true;
}

bool SpecificItem::isSpecificCallAfterReturnRequired() const
{
    return callAfterReturn;
}
