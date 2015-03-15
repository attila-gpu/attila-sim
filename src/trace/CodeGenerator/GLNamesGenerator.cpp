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

#include "GLNamesGenerator.h"
#include <iostream>
#include <cstring>

using namespace std;

GLNamesGenerator::GLNamesGenerator( const FuncExtractor& fe, const ConstExtractor& ce, 
                                   const char* oDir1, const char* oDir2 ) : fe(fe), ce(ce)
{
    if ( oDir2 )
    {
        this->oDir2 = new char[strlen(oDir2)+1];
        strcpy(this->oDir2, oDir2);
    }
    else
        this->oDir2 = NULL;

    setOutputDir(oDir1);
}

GLNamesGenerator::~GLNamesGenerator()
{
    if ( oDir2 )
        delete[] oDir2;
}

void GLNamesGenerator::dumpInfo() const
{
    cout << "GLNamesGenerator generates:\n  { GLFunctionNames.gen, GLConstantNames.gen, APICall.gen, opengl32.def }";
}

bool GLNamesGenerator::generateGLConstantNames( std::ostream& f, const ConstExtractor& ce )
{
    int i;
    int nConsts = ce.count();
    const ConstDescription* cd;
    for ( i = 0; i < nConsts; i++ )
    {
        cd = ce.getConstant(i);
        f << "{\"" << cd->getString() << "\", 0x" << hex << cd->getValue() << dec << "}";
        if ( i < nConsts - 1 )
            f << ",";
        f << endl;
    }
    return true;
}

bool GLNamesGenerator::generateGLFunctionNames( std::ostream& f, const FuncExtractor& fe )
{
    int i, nFuncs;
    
    nFuncs = fe.count();
    for ( i = 0; i < nFuncs; i++ )
    {
        f << "\"" << fe.getFunction(i)->getName() << "\"";
        if ( i < fe.count() - 1)
            f << "," << endl;
        else
            f << endl;
    }
    return true;

}

bool GLNamesGenerator::generateGLFunctionNamesList( std::ostream& f, const FuncExtractor& fe, bool withCommas, const char* prefix )
{
    int i, nFuncs;
    
    nFuncs = fe.count();
    for ( i = 0; i < nFuncs; i++ )
    {
        f << prefix << fe.getFunction(i)->getName();
        if ( withCommas && i < fe.count() - 1)
            f << "," << endl;
        else
            f << endl;
    }
    return true;

}


bool GLNamesGenerator::generateCode()
{
    ofstream f;

    /*************************************
     * Generate GLFunctionNames.gen file *
     *************************************/
    if ( openOFile(f, "GLFunctionNames.gen") )
    {    
        writePreface(f);

        generateGLFunctionNames(f, fe);

        f.close();
    }

    /*************************************
     * Generate GLConstantNames.gen file *
     *************************************/
    if ( openOFile(f, "GLConstantNames.gen") )
    {    
        writePreface(f);

        generateGLConstantNames(f, ce);

        f.close();
    }
    
    /*********************************
     * Generate JumpTableWrapper.gen *
     *********************************/

    if ( openOFile(f, "JumpTableWrapper.gen") )
    {
        writePreface(f);

        generateGLFunctionNamesList(f, fe, true);
    
        f.close();
    }


    /******************************
     * Generate APICall.gen file *
     ******************************/
    if ( openOFile(f, "APICall.gen") )
    {

        writePreface(f);

        f << "#ifndef APICALL_GEN\n    #define APICALL_GEN\n\n";
    
        /* commas and prefix */
        generateGLFunctionNamesList(f, fe, true, "APICall_");

        f << "\n#endif // APICALL_GEN\n";
    
        f.close();
    }

    /******************************
     * Generate opengl32.def file *
     ******************************/
    setOutputDir(oDir2);

    if ( openOFile(f, "opengl32.def") )
    {
        f << "LIBRARY opengl32.dll" << endl;
        f << "EXPORTS" << endl;
    
        generateGLFunctionNamesList(f, fe, false); /* without commas */

        f.close();
    }
    return true;
}
