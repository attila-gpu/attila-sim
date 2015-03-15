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

#include "GLIEntryPointsGenerator.h"

using namespace std;

GLIEntryPointsGenerator::GLIEntryPointsGenerator(const FuncExtractor& fe, 
                                               string outputDir) : fe(fe)
{
    setOutputDir(outputDir.c_str());
}


void GLIEntryPointsGenerator::dumpInfo() const
{
    cout << "GLIEntryPointsGenerator generates:\n  { GLIEntryPointsGenerator.h, GLIEntryPointsGenerator.cpp }";
}

vector<string> GLIEntryPointsGenerator::generateHeader(ostream& out, const FuncDescription& fd)
{
    int i, iParam;

    ParamDescription* pd;

    char temp[256];
    vector<string> paramNames;

    if ( fd.getReturn() != NULL )
        out << fd.getReturn()->toString() << " ";
    else
        out << "void ";

    out << "GLAPIENTRY ";
    out << fd.getName() << "(";
    iParam = fd.countParams();

    for ( i = 0; i < iParam; i++ )
    {
        pd = fd.getParam(i);
        
        if ( pd->isConst() )
            out << "const ";
        
        out << pd->getType() << " ";
        
        if ( pd->isPointer() || pd->isDoublePointer() ) 
        {   
            if ( pd->isPointer() )
                out << "*";
            else // double pointer
                out << "**";
        }
        
        if ( pd->getName() == NULL ) // create names for variables with type but without name
        {
            sprintf(temp,"_p%d",i);
            paramNames.push_back(temp);
        }
        else
            paramNames.push_back(pd->getName());           

        out << paramNames.back();;
        
        if ( i < iParam - 1 )
            out << ", ";
    }

        

    return paramNames;

}

bool GLIEntryPointsGenerator::generateEntryPointCall(std::ostream& out, 
                                                    std::string gliName, 
                                                    const FuncDescription& fd)
{
    vector<string> paramNames = generateHeader(out, fd); 

    out << ")\n{" << endl;
    out << "    " << gliName << ".init();\n";

    ParamDescription* pd = fd.getReturn();
    if ( pd != 0 )
    {
        if ( pd->isConst() )
            out << "    const ";
        else
            out << "    ";
        out << pd->getType() << " ";
        if ( pd->isPointer() )
            out << "*";
        out << "_result;\n";
    }

    if ( fd.getName() == string("wglSwapBuffers") )
    {
        out << "    " << gliName << ".setHDC(" 
            << fd.getParam(0)->getName() << ");\n";
        out << "    if ( " << gliName << ".isForcedSwap() )\n"
            "    {\n        _result = ";
            out << "glCalls().";
            generateFuncCall(out, fd.getName(), "", paramNames, paramNames.size());
            out << ";\n";
            out << "        return _result;\n    }\n";
    }

    out << "    PREV_USER_CALL(" << fd.getName() << ",";
    generateFuncCall(out, fd.getName(), "", paramNames, paramNames.size());
    out << ")\n";


    if ( pd != 0 )
    {
        out << "    _result = ";
    }
    else
        out << "    ";

    out << "glCalls().";
    generateFuncCall(out, fd.getName(), "", paramNames, paramNames.size());
    out << ";\n";

    string fName = fd.getName();

    if ( fName.substr(0, 8) == string("glVertex") )
    {
        if ( fName.size() > 8 )
        {
            if ( fName.at(8) >= '0' && fName.at(8) <= '4' )
            {
                out << "    " << gliName << ".incVertexCount();\n";
            }
        }
    }

    if ( fName == "glDrawArrays" )
    {
        out << "    " << gliName << ".incVertexCount(" << paramNames[2] 
            << "-" << paramNames[1] << ");\n";
    }
    else if ( fName == "glMultiDrawArrays" )
    {
        out << "    {\n";
        out << "        for ( int i = 0; i < " << paramNames[3] << "; i++ )\n";
        out << "            " << gliName << ".incVertexCount("
            << paramNames[2] << "[i]-" << paramNames[1] << "[i]);\n";
        out << "    }\n";
    }
    else if ( fName == "glDrawElements" )
    {
        out << "    " << gliName << ".incVertexCount(" << paramNames[1] << ");\n";
    }
    else if ( fName == "glDrawRangeElements" )
    {
        out << "    " << gliName << ".incVertexCount(" << paramNames[3] << ");\n";
    }
    else if ( fName == "glMultiDrawElements" )
    {
        out << "    {\n";
        out << "        for ( int i = 0; i < " << paramNames[4] << "; i++ )\n";
        out << "            " << gliName << ".incVertexCount("
            << paramNames[1] << "[i]);\n";
        out << "    }\n";
    }



    out << "    POST_USER_CALL(" << fd.getName() << ",";
    generateFuncCall(out, fd.getName(), "", paramNames, paramNames.size());
    out << ")\n";

    if ( fName == "glEnd" || 
         fName == "glDrawArrays" || 
         fName == "glMultiDrawArrays" ||
         fName == "glDrawElements" ||
         fName == "glDrawRangeElements" ||
         fName == "glMultiDrawElements" )
    {
        out << "    " << gliName << ".doEndBatchEvent();\n";
    }


    // End frame action
    if ( fName == string("wglSwapBuffers") )
    {
        out << "    " << gliName << ".doEndFrameEvent();\n";
    }

    if ( pd != 0 )
    {
        out << "    return _result;\n";
    }

    out << "}\n";
    return true;
}


bool GLIEntryPointsGenerator::generateAllEntryPointCalls(std::ostream& out, 
                                                        std::string gliName,
                                                        const FuncExtractor& fe)
{
    int i, nFuncs;
    nFuncs = fe.count();    
    for ( i = 0; i < nFuncs; i++ )
    {
        generateEntryPointCall(out, gliName, *fe.getFunction(i));
        out << endl;
    }
    return true;
}

bool GLIEntryPointsGenerator::generateCode()
{
    ofstream f;
    if ( openOFile(f,"GLIEntryPoints.h") )
    {
        writePreface(f);
        f << "#ifndef GLIENTRYPOINTS_H\n";
        f << "    #define GLIENTRYPOINTS_H\n\n";
    
        f << "#include \"gl.h\"\n";
        f << "#include \"wglext.h\"\n";
        f << "#include \"MainGLInstrument.h\"\n\n";

        f << "#define WGLAPI GLAPI\n\n";

        generateFuncDeclarations(f, fe);

        f << "\n#endif // GLIENTRYPOINTS_H\n";
        f.close();
    }
    if ( openOFile(f, "GLIEntryPoints.cpp") )
    {
        writePreface(f);
        f << "#include \"GLIEntryPoints.h\"\n";
        generateAllEntryPointCalls(f, "gli()", fe);
        f.close();
    }

    return true;
}


