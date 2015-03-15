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

#include "StringTokenizer.h"
#include <cstring>
#include <ostream>
#include <iostream>

using namespace std;

#define DEFAULT_DELIMITERS " \t"


StringTokenizer::~StringTokenizer()
{
    delete[] str;
    delete[] delimiters;
}

StringTokenizer::StringTokenizer( const char* str ) : pos(0)
{
    this->str = new char[(strLen = strlen(str))+1];
    strcpy(this->str, str);
    
    delimiters = new char[3];
    strcpy(delimiters, DEFAULT_DELIMITERS);
}

StringTokenizer::StringTokenizer( const char* str, int size ) : pos(0)
{
    if ( str[size-1] == '\0' ) {
        this->str = new char[size];
        strLen = size - 1;
    }
    else 
    {
        this->str = new char[size+1];
        this->str[size] = '\0';
        strLen = size;
    }
    
    strncpy(this->str, str, size);        
    
    delimiters = new char[3];    
    strcpy(delimiters, DEFAULT_DELIMITERS);
}

bool StringTokenizer::isDelimiter( const char* delimiters, char c )
{
    int i = 0;    
    while ( delimiters[i] && c != delimiters[i] )
        i++;
    return ( delimiters[i] != '\0' );
}


const char* StringTokenizer::getToken( char* buffer, int size, const char *delimiters )
{    
    const char* delims;
    
    if ( delimiters == NULL ) 
        delims = this->delimiters;
    else 
        delims = delimiters;
        
    int i = 0;

    // skip delimiters
    while ( pos < strLen && isDelimiter(delims, str[pos]) )
        pos++;

    if ( pos == strLen )
        return NULL; // no more tokens availables

    while (  pos < strLen && i < size && !isDelimiter(delims, str[pos]) )
        buffer[i++] = str[pos++];

    buffer[i] = '\0';
    
    return buffer;
}

const char* StringTokenizer::getToken( char* buffer, int size )
{
    return getToken(buffer, size, NULL);
}

char StringTokenizer::getCurrentChar() const
{
    return str[pos];
}

int StringTokenizer::getPos() const
{
    return pos;
}


int StringTokenizer::skipWhileChar( const char* cList )
{    
    while ( pos < strLen && isDelimiter(cList, str[pos]) )
        pos++;
    if ( pos == strLen ) 
    {
        pos--;
        return strLen;
    }
    return pos;
}

int StringTokenizer::skipWhileCharRev( const char* cList )
{    
    while ( pos >= 0 && isDelimiter(cList, str[pos]) )
        pos--;
    if ( pos < 0 )
    {
        pos = 0;
        return -1;
    }
    return pos;
}


int StringTokenizer::skipUntilChar( const char* cList )
{
    while ( pos < strLen && !isDelimiter(cList,str[pos]) )
        pos++;
    if ( pos == strLen )
    {
        pos--;;
        return strLen;
    }
    return pos;
}

int StringTokenizer::skipUntilCharRev( const char* cList )
{
    while ( pos >= 0 && !isDelimiter(cList,str[pos]) )
        pos--;
    if ( pos < 0 )
    {
        pos = 0;
        return strLen;
    }
    return pos;
}


bool StringTokenizer::isCurrentChar( const char* cList ) const
{
    //int len = strlen(cList);
    return isDelimiter(cList, str[pos]);
}

int StringTokenizer::skipChars( int howMany )
{
    pos += howMany;

    if ( pos < 0 )
        pos = 0;
    else if ( pos >= strLen )
        pos = strLen-1;    
    
    return pos;
}


int StringTokenizer::setPos( int newPos )
{
    if ( newPos > 0 && newPos < strLen )
        return (pos = newPos);
    else 
    { // clamp
        if ( newPos < 0 )
            return (pos = 0);
        else
            return (pos = (strLen - 1));
    }
}

int StringTokenizer::length() const
{
    return strLen;
}

void StringTokenizer::dump() const
{
    cout << str << endl;
}

char StringTokenizer::getChar( int absolutePos )
{
    if ( 0 <= absolutePos && absolutePos < strLen )
        return str[absolutePos];
    return 0;
}

void StringTokenizer::setDefaultDelimiters( const char* delims )
{
    if ( delimiters != NULL )
        delete[] delimiters;
    
    delimiters = new char[strlen(delims)+1];
    strcpy(delimiters,delims);
}
