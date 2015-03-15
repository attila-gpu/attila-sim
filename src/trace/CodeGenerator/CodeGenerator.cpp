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

#include "CodeGenerator.h"
#include <cstring>
#include "support.h"
#include <iostream>
#include <cstdio>

using namespace std;


CodeGenerator::CodeGenerator() : isNull(true)
{
}

CodeGenerator::~CodeGenerator()
{
}

void CodeGenerator::dumpInfo() const
{
    cout << "No info available for this CodeGenerator";
}

FuncGenerator::~FuncGenerator()
{
    /* empty */
}

bool FuncGenerator::generateFuncDeclarations( ostream& f, const FuncExtractor& fe )
{
    int i, j, nFuncs, nParams;
    const FuncDescription* fd;    
    
    nFuncs = fe.count();
    for ( i = 0; i < nFuncs; i++ )
    {
        fd = fe.getFunction(i);
        if ( fd->getMacro1() )
            f << fd->getMacro1() << " ";

        f << ( fd->getReturn() == NULL ? "void" : fd->getReturn()->toString() ) << " ";

        if ( fd->getMacro2() )
            f << fd->getMacro2() << " ";
        
        f << fd->getName() << "( ";
        
        nParams = fd->countParams();
        for ( j = 0; j < nParams; j++ )
        {
            f << fd->getParam(j)->toString();
            if ( j < nParams - 1 )
                f << ", ";            
        }
        f << " );\n";

    }
    return true;

}

bool CodeGenerator::openOFile( ofstream& f, const char* fileName )
{
    char oFile[256];

    if ( isNull )
        return false;

    /*****************************
     * Generate GLWrapper.h file *
     *****************************/
    strcpy(oFile, oDir);
#ifdef WIN32
    strcat(oFile,"\\");
#else
    strcat(oFile,"/");
#endif
    strcat(oFile, fileName);

    f.open(oFile);
    
    if ( !f.is_open() )
    {
        char msg[256];
        sprintf(msg, "Output file: %s could not be opened (directory does not exist?)", oFile);
        panic("CodeGenerator","openOFile()", msg);
    }

    return true;
}

const char* CodeGenerator::getOutputDir() const
{
    return oDir;
}

void CodeGenerator::setOutputDir( const char* path )
{
    if ( path == 0 )
        isNull = true;
    else
    {
        isNull = false;
        strcpy(oDir,path);
    }
}


void CodeGenerator::writePreface( ostream& f)
{
    f << "/////////////////////////////////////////////////\n"
         "// This file has been automatically generated  //\n"
         "// Do not modify this file                     //\n"
         "/////////////////////////////////////////////////\n" << endl;
}




const char* FuncGenerator::knownTypes[] =
{
    "GLenum",
    "GLboolean",
    "GLbitfield",
    "GLvoid",
    "void",
    "GLbyte",
    "char",
    "unsigned char",
    "GLubyte",
    "GLshort",
    "short",
    "GLushort",
    "unsigned short",
    "GLint",
    "int",
    "GLuint",
    "unsigned int",
    "long",
    "unsigned long",
    "GLfloat",
    "float",
    "GLdouble",
    "double",
    "GLclampf",
    "GLclampd",
    "GLsizei",
    "GLintptrARB",
    "GLintptr",
    "GLsizeiptrARB",
    "GLsizeiptr",
    /* ARB new types */
    "GLhandleARB", /* from glext.h : typedef unsigned int GLhandleARB; */
    "GLcharARB", /* from glext.h : typedef char GLcharARB; */
    /* Windows specific */
    "HDC",
    "HGLRC"
    //"PIXELFORMATDESCRIPTOR"

};

bool FuncGenerator::isKnownType( const char* type )
{
    int i;
    for ( i = 0; i < sizeof(knownTypes)/sizeof(knownTypes[0]); i++ )
    {
        if ( EQ(type,knownTypes[i]) )
            return true;
    }
    return false;

}

bool FuncGenerator::generateFuncCall( ostream& f, const char* fName, const char* postfix,
                                    char paramNames[20][64], int nParams )
{
    int i;
    f << fName << ( postfix != NULL ? postfix : "" ) << "(";        
    
    for ( i = 0; i < nParams; i++ )
    {        
        f << paramNames[i];
        if ( i < nParams - 1 )
            f << ",";
    }
    f << ")";
    return true;
}

bool FuncGenerator::generateFuncCall( std::ostream& f,
                                      std::string fName, 
                                      std::string postfix, 
                                      const std::vector<std::string>& paramNames, 
                                      int nParams)
{
    int i;
    f << fName.c_str() << postfix.c_str() << "(";
    
    for ( i = 0; i < nParams; i++ )
    {        
        f << paramNames[i].c_str();
        if ( i < nParams - 1 )
            f << ",";
    }
    f << ")";
    return true;
}


void FuncGenerator::createCast( ParamDescription* pd, char buffer[] )
{
    buffer[0] = '\0';
    if ( pd->isConst() )
        strcat(buffer, "const ");
    strcat(buffer, pd->getType());
    strcat(buffer, " ");
    if ( pd->isPointer() )
        strcat(buffer, "*");
    else if ( pd->isDoublePointer() )
        strcat(buffer, "**");    
}
