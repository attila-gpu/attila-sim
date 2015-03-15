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

#include "FuncDescription.h"
#include <cstring>
#include <ostream>
#include <iostream>
#include "StringTokenizer.h"

using namespace std;

//#define DELIM_CHARS " \t),"

#define EQ(s1,s2) ( strcmp((s1),(s2)) == 0 )

#define TOSTRING_BUFFER_SIZE 2048
char FuncDescription::toStringBuffer[TOSTRING_BUFFER_SIZE];


FuncDescription::FuncDescription() : nParams(0), name(NULL), macro1(NULL), macro2(NULL)
{
    // empty
}

bool FuncDescription::isMacro1( const char* candidate )
{
    static const char* macros1[] =
    {
        "GLAPI",
        "WGLAPI",
        "WINGDIAPI",
        "extern"
    };
    int nMacros = sizeof(macros1)/sizeof(macros1[0]);
    for ( int i = 0; i < nMacros; i++ )
    {
        if ( EQ(candidate,macros1[i]) )
            return true;
    }
    return false;
}


bool FuncDescription::isMacro2( const char* candidate )
{
    static const char* macros2[] =
    {
        "GLAPIENTRY",
        "APIENTRY",
        "WINAPI"
    };
    int nMacros = sizeof(macros2)/sizeof(macros2[0]);
    for ( int i = 0; i < nMacros; i++ )
    {
        if ( EQ(candidate,macros2[i]) )
            return true;
    }
    return false;
}

bool FuncDescription::isVoid( const char* candidate )
{
    static const char* voids[] =
    {
        "GLvoid",
        "void",
        "VOID"
    };

    int nVoids = sizeof(voids)/sizeof(voids[0]);
    for ( int i = 0; i < nVoids; i++ )
    {
        if ( EQ(candidate,voids[i]) )
            return true;
    }
    return false;
}

FuncDescription::~FuncDescription()
{
    delete[] macro1;
    delete[] macro2;
    delete[] name;
    
    for ( int i = 0; i < nParams; i++ )
        delete param[i];    
}


FuncDescription* FuncDescription::parseFunc( int& i, const char* line, int lineLen )
{

#define DELIM_CHARS " \t(),;"

#define GET_TOKEN\
    {\
        if ( !st.getToken(token,sizeof(token),DELIM_CHARS) ) {\
            delete fd;\
            return NULL;\
        }\
    }

    

    int temp, temp2;

    char token[16384];

    char retType[16384] = "";

    StringTokenizer st(&line[i], lineLen - i);    

    st.skipWhileChar(" \t\n"); // skip whitespaces
    if ( st.isCurrentChar("#") ) // directive detected, skip
        return NULL;

    FuncDescription* fd = new FuncDescription;

    //st.dump();

    GET_TOKEN;

    if ( token[0] == '/' && token[1] == '/' ) // skip line with comments
    {
        /*
        cout << "Skip line: ";
        st.dump();
        */
        delete fd;
        return NULL;
    }

    if ( isMacro1(token) ) // Skip GLAPI or WGLAPI if it is found
    {
        // if macro exist, store it in macro1 variable
        fd->macro1 = new char[strlen(token)+1];
        strcpy(fd->macro1,token);
        GET_TOKEN;
    }    
    
    temp = st.getPos(); // get position after first type token
    temp2 = st.skipUntilChar("("); // search function name

    if ( temp2 == st.length() ) 
    {   // char '(' not found, abort
        delete fd;
        return NULL;
    }

    st.skipChars(-1); // skip "("
    temp2 = st.skipWhileCharRev(" \t\n"); // skip whites after name 
    if ( temp2 < temp )
    {
        // name not found! ( it is not a function definition )
        delete fd;
        return NULL;
    }
    
    temp2 = st.skipUntilCharRev(" \t\n"); // temp2 points previous character to name function

    // positions after first type token
    st.setPos(temp);

    while ( temp <= temp2 )
    {        
        if ( isMacro2(token) )
        {
            // if macro exist, store it in macro2 variable
            fd->macro2 = new char[strlen(token)+1];
            strcpy(fd->macro2, token);
        }
        else
        {
            strcat(retType, token);
            strcat(retType, " ");
        }
        GET_TOKEN;
        st.skipWhileChar(" \t\n"); // skip whites
        temp = st.getPos();   
    }
    
    // last token can be name or a previous macro GLAPIENTRY or APIENTRY
    if ( isMacro2(token) )
    {
        // if macro exist, store it in macro2 variable ( assume that can be two macro2 candidates )
        fd->macro2 = new char[strlen(token)+1];
        strcpy(fd->macro2, token);
        GET_TOKEN;
    }

    strcat(retType, ","); // mark parameter end to allow correct parsing   
        
    temp = 0;

    fd->returnType = ParamDescription::parseParam(temp, retType, (int) strlen(retType));        

    if ( fd->returnType == NULL ) 
    {
        delete fd;
        return NULL;
    }

    if ( isVoid(fd->returnType->getType()) &&
         !fd->returnType->isPointer() && 
         !fd->returnType->isDoublePointer())
    {
        delete fd->returnType;
        fd->returnType = NULL;
    }

    fd->name = new char[strlen(token)+1];
    strcpy(fd->name, token);   

    st.skipWhileChar(" \t(");

    int iParam = 0;
    ParamDescription** pd = fd->param;    

    int iNew = i;    
    iNew = st.getPos();

    while ( st.getCurrentChar() != ')' )
    {                    
        pd[iParam] = ParamDescription::parseParam(iNew, line, lineLen);
        if ( pd[iParam] == NULL )
        {
            // not a function definition
            // for example: "#  define GLAPIENTRY __stdcall"            
            //st.dump();
            fd->nParams = iParam; // for allowing correct destruction
            delete fd;
            return NULL;
        }        
        else {
            // ignore void type definitions, for example: glEnd(void)
            if ((EQ(pd[iParam]->getType(),"GLvoid") || EQ(pd[iParam]->getType(),"void")) &&
                  !pd[iParam]->isPointer() && !pd[iParam]->isDoublePointer())
                delete pd[iParam];
            else
                iParam++;
        }
        
        st.setPos(iNew);                
    } 
    
    // Check for ';'
    st.skipChars(1); // skip ')'
    st.skipWhileChar(" \t\n");    
    if ( st.getCurrentChar() != ';' )
    {
        delete fd;
        return NULL;
    }
    
    fd->nParams = iParam; 
    
    i += st.getPos();
    
    return fd;
}



FuncDescription* FuncDescription::parseFunc( const char* line, int lineLen )
{
    int dummy = 0;
    return parseFunc(dummy, line, lineLen );
}

ParamDescription* FuncDescription::getReturn() const
{
    return returnType;
}

ParamDescription* FuncDescription::getParam( int i ) const
{
    if ( 0 > i || i >= nParams)
        return NULL;
    
    return param[i];
}

void FuncDescription::dump( bool debug ) const
{
    int i;
    if ( debug )
    {
        cout << "Func name: " << "\"" << ( name != NULL ? name : "NULL" ) << "\"" << endl;

        cout << "Return type: " << endl;
        if ( returnType == NULL )
            cout << "void";
        else
            returnType->dump(true);

        cout << "Parameters: " << endl;

        for ( i = 0; i < nParams; i++ )
        {
            cout << "param " << i << ":" << endl;
            param[i]->dump(true);
        }
    }
    else 
    {
        if ( returnType == NULL )
            cout << "void";
        else
            returnType->dump();
        if ( !returnType->isPointer() && !returnType->isDoublePointer() )
            cout << " ";
        cout << name << "( ";
        for ( i = 0; i < nParams; i++ ) 
        {
            param[i]->dump();
            if ( i < nParams - 1 )
                cout << ", ";
        }
        cout << " )";        
    }
}

const char* FuncDescription::toString( char* buffer, int size ) const
{
    char temp[1024];
    char paramStr[256];

    if ( returnType == NULL )
        strcpy(temp,"void ");
    else
    {
        strcpy(temp, returnType->toString(paramStr,256));
        if ( !returnType->isPointer() && !returnType->isDoublePointer() )
            strcat(temp, " ");
    }

    strcat(temp, name);
    strcat(temp, "( ");
    for ( int i = 0; i < nParams; i++ )
    {
        param[i]->toString(paramStr,256);
        strcat(temp, paramStr);
        if ( i < nParams - 1 )
            strcat( temp, ", " );
    }
    strcat(temp, " )");
    if ( (int)strlen(temp) < size )
    {
        strcpy(buffer, temp);
        return buffer;
    }
    return NULL;
}

const char* FuncDescription::toString() const
{
    return toString(toStringBuffer, TOSTRING_BUFFER_SIZE);
}


const char* FuncDescription::getName() const
{
    return name;
}


int FuncDescription::countParams() const
{
    return nParams;
}


const char* FuncDescription::getMacro1() const
{
    return macro1;
}

const char* FuncDescription::getMacro2() const
{
    return macro2;
}
