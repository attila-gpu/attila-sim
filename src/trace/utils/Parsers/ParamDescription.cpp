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

#include "ParamDescription.h"
#include <cstring>
#include <iostream>
#include <istream>
#include "StringTokenizer.h"

using namespace std;

#ifndef EQ
    #define EQ(s1,s2) (strcmp((s1),(s2)) == 0)
#endif

#define TOSTRING_BUFFER_SIZE 1024
char ParamDescription::toStringBuffer[TOSTRING_BUFFER_SIZE];

// private
ParamDescription::ParamDescription() : 
    isPointerFlag(false), 
    isDoublePointerFlag(false),
    isConstFlag(false),
    name(NULL),
    type(NULL)
{
    // empty
}

/*
 * between <> is optional
 *
 * < > -> indicates one or more optional whitespaces
 * 
 * <const> type< ><*>< ><*>< >name<[<NUMBER>]>
 */
ParamDescription* ParamDescription::parseParam(int&  i, const char* line, int lineLen )
{

#define DELIM_CHARS " \t(),;*"

#define GET_TOKEN\
    {\
        if ( !st.getToken(token,sizeof(token),DELIM_CHARS) ) {\
            delete param;\
            return NULL;\
        }\
    }
    
    int temp;
    
    char token[16384];

    int indirections = 0;

    StringTokenizer st(&line[i], lineLen - i);

    ParamDescription* param = new ParamDescription;
    
    int j = 1;

    //************************
    //* CHECK const MODIFIER *
    //************************
    GET_TOKEN;
    
    if ( EQ(token,"const") )
    {
        param->isConstFlag = true;
        GET_TOKEN;
    }
    
    //************************
    //* PARSE PARAMETER TYPE *
    //************************
    char tempType[16384]; // store temporary type
    strcpy(tempType, token); // copy first "type" token

    //******************************************
    //* CHECK unsigned OR signed type modifier *
    //******************************************
    
    if ( EQ(token,"unsigned") || EQ(token,"signed") )
    {
        GET_TOKEN;
        strcat(tempType," ");
        strcat(tempType,token);
    }

    param->type = new char[strlen(tempType)+1];
    strcpy(param->type,tempType);

    // Computer indirections
    while ( st.isCurrentChar(" \t*") )
    {
        if ( st.getCurrentChar() == '*' )
            indirections++;
        st.skipChars(1);
    }

    if ( st.isCurrentChar(",)") ) // no name found
    {
        if ( indirections == 1 )
            param->isPointerFlag = true;
        else if ( indirections == 2 )
            param->isDoublePointerFlag = true;
        else if ( indirections > 2 )
        {
            cout << "ParamDescription. Unexpected. Triple indirection found!";
            delete param;
            return NULL;
        }
        i += st.getPos();
        return param;
    }

    GET_TOKEN;

    //************************
    //* PARSE PARAMETER NAME *
    //************************
    temp = 0;    
    while ( token[temp] != '\0' && token[temp] != '[' )
        temp++;

    if ( token[temp] == '[' )
    {   
        indirections++;
        token[temp] = '\0';                        
    }

    param->name = new char[temp+1];
    strcpy(param->name, token);
 
    st.skipWhileChar( " \t" );

    if ( st.getCurrentChar() == ',' || st.getCurrentChar() == ')' )
    {
        if ( indirections == 1 )
            param->isPointerFlag = true;
        else if ( indirections == 2 )
            param->isDoublePointerFlag = true;
        else if ( indirections > 2 )
        {
            cout << "ParamDescription. Unexpected. Triple indirection found!";
            delete param;
            return NULL;
        }
        i += st.getPos();
        return param;
    }

    return NULL;
}


ParamDescription::PD_TYPE ParamDescription::parseParamType( int& i, const char* paramString, int paramStringLen )
{    
    char* token = new char[paramStringLen+1-i];
    strcpy( token, paramString+i );    
    
    PD_TYPE type = UNKNOWN;    
    
    if ( EQ("GLenum",token) ) type = ENUM;
    else if (EQ("GLboolean",token) || EQ("bool",token) || EQ("BOOL",token)) type = BOOLEAN;
    else if (EQ("GLbitfield",token)) type = BITFIELD;
    else if (EQ("GLvoid",token) || EQ("void",token) || EQ("VOID",token)) type = VOID;
    else if (EQ("GLbyte",token) || EQ("char",token)) type = BYTE;
    else if (EQ("GLubyte",token) || EQ("unsigned char",token)) type = UBYTE;
    else if (EQ("GLshort",token) || EQ("short",token) || EQ("unsigned short",token)) type = SHORT;
    else if (EQ("GLushort",token) || EQ("unsigned short",token)) type = USHORT;
    else if (EQ("GLint",token) || EQ("int",token) || EQ("long",token)) type = INT;
    else if (EQ("GLuint",token) || EQ("unsigned int",token) || EQ("unsigned long",token)) type = UINT;
    else if (EQ("GLfloat",token) || EQ("float",token)) type = FLOAT;
    else if (EQ("GLdouble",token) || EQ("double",token)) type = DOUBLE;
    else if (EQ("GLclampf",token)) type = CLAMPF;
    else if (EQ("GLclampd",token)) type = CLAMPD;
    else if (EQ("GLsizei",token)) type = SIZEI;
    else if (EQ("GLintptrARB",token) || EQ("GLintptr",token)) type = INT;
    else if (EQ("GLsizeiptrARB",token) || EQ("GLsizeiptr",token)) type = SIZEI;
    else if (EQ("GLhandleARB", token) ) type = UINT;
    else if (EQ("GLcharARB", token) ) type = BYTE;
    else if (EQ("HDC", token) ) type = ParamDescription::HDC;
    else if (EQ("PIXELFORMATDESCRIPTOR", token) ) type = ParamDescription::PFD;

    delete[] token;

    return type;
}

inline ParamDescription::PD_TYPE ParamDescription::parseParamType( const char* paramString, int paramStringLen )
{
    int i = 0;
    return parseParamType(i, paramString, paramStringLen);
}

ParamDescription::PD_TYPE ParamDescription::parseParamType() const
{
    int i = 0;    
    return parseParamType(i, type, (int) strlen(type));
}

ParamDescription::~ParamDescription()
{
    delete[] name;
    delete[] type;
}

void ParamDescription::dump( bool debug ) const
{
    if ( debug )
    {
        cout << "Name: " << "\"" << ( name != NULL ? name : "NULL" ) << "\"" << endl;
        cout << "Type: " << "\"" << type << "\"" << endl;
        cout << "PD_TYPE: " << this->parseParamType() << endl;
        cout << "Pointer modif: " << ( isPointerFlag ? "*" : ( isDoublePointerFlag ? "**" : "NONE" )) << endl;    
        cout << "HasConstModif: " << ( isConstFlag ? "TRUE" : "FALSE" ) << endl;
    }
    else 
    {
        if ( isConstFlag )
            cout << "const ";
        cout << type;
        if ( isPointerFlag )
            cout << " *" << ( name ? name : "" );
        else if ( isDoublePointerFlag )
            cout << " **" << ( name ? name : "" );
        else if ( name )
            cout << " " << name;
            
    }
}

const char* ParamDescription::toString( char* buffer, int size ) const
{
    char temp[256];
    
    if ( isConstFlag )
        strcpy( temp,"const ");
    else
        temp[0] = '\0';

    strcat( temp, type );
    
    if ( isPointerFlag )
    {
        strcat( temp, " *");
        if ( name )
            strcat( temp, name );
    }
    else if ( isDoublePointerFlag )
    {
        strcat( temp, " **");
        if ( name )
            strcat( temp, name );
    }
    else {
        if ( name ) 
        {
            strcat(temp, " ");
            strcat(temp, name);
        }
    }

    if ( (int)strlen(temp) < size )
    {
        strcpy( buffer, temp );
        return buffer;
    }
    
    return NULL;
}

const char* ParamDescription::toString() const
{
    return toString(toStringBuffer, TOSTRING_BUFFER_SIZE);
}

const char* ParamDescription::getName() const
{
    return name;
}

const char* ParamDescription::getType() const
{
    return type;
}

bool ParamDescription::isPointer() const
{
    return isPointerFlag;
}

bool ParamDescription::isDoublePointer() const
{
    return isDoublePointerFlag;
}


bool ParamDescription::isConst() const
{
    return isConstFlag;
}
