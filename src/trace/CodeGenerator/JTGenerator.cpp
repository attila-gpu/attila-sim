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

#include "JTGenerator.h"
#include <iostream>
#include <cstring>

using namespace std;

JTGenerator::JTGenerator( const FuncExtractor& fe, const FuncExtractor& libFuncsSL, const char* oDir, const char* jtName )
: CodeGenerator(), fe(fe), libFuncsSL(libFuncsSL)
{
    setOutputDir(oDir);
    strcpy(this->jtName, jtName);
}

void JTGenerator::dumpInfo() const
{
    cout << "JTGenerator generates:\n  { GLJumpTableFields.gen, GLJumpTableSL.gen }";
}

bool JTGenerator::generateJTFields( std::ostream& fout, const FuncExtractor& fe )
{
    int i, j;

    for ( i = 0; i < fe.count(); i++ )
    {
        // return type
        const FuncDescription* fd = fe.getFunction(i);
        if ( fd->getReturn() == NULL )
            fout << "void";
        else
            fout << fd->getReturn()->toString();
        // function name
        fout << " (GLAPIENTRY *" << fd->getName() << ") (";

        // parameters
        for ( j = 0; j < fd->countParams(); j++ )
        {
            ParamDescription* pd = fd->getParam(j);
            if ( pd->isConst() )
                fout << "const ";
            fout << pd->getType() << " ";
            if ( pd->isPointer() )
                fout <<"*";
            else if ( pd->isDoublePointer() )
                fout << "**";

            if ( j < fd->countParams() - 1 )
                fout << ",";
        }
        fout << ");" << endl;
    }
    return true;
}

bool JTGenerator::generateJumpTableStaticInit( std::ostream& f, const FuncExtractor& libFuncsSL, 
                                               const char* jtName )
{
    int i;
    int nFuncs = libFuncsSL.count();
    const char* fdName;
    for ( i = 0; i < nFuncs; i++ )
    {
        fdName = libFuncsSL.getFunction(i)->getName();
        f << jtName << "." << fdName << " = " << fdName << ";" << endl;
    }
    
    return true;
}


bool JTGenerator::generateCode()
{
    ofstream f;
    /***************************************
     * Generate GLJumpTableFields.gen file *
     ***************************************/
    if ( openOFile(f, "GLJumpTableFields.gen") )
    {
        writePreface(f);

        generateJTFields(f,fe);

        f.close();
    }

    /***************************************
     * Generate GLJumpTableSL.gen file *
     ***************************************/
    if ( openOFile(f, "GLJumpTableSL.gen") )
    {
        writePreface(f);

        generateJumpTableStaticInit(f, libFuncsSL, jtName);

        f.close();
    }
    
    return true;
}

