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

#include "StubGenerator.h"
#include "support.h"
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

StubGenerator::StubGenerator( const FuncExtractor& fe, const char* oDir ) : fe(fe)
{
    setOutputDir(oDir);
}

void StubGenerator::dumpInfo() const
{
    cout << "StubGenerator generates:\n  { StubApiCalls.h, StubApiCalls.cpp }";
}

bool StubGenerator::checkSpecialCall(const char* fn)
{

    /*
#ifdef WIN32
    if ( strcmp(fn, "wglSetPixelFormat") == 0 )
        return true;
#endif
        */
    return false; // for UNIX ATILA simulator compatibility
}

/* Version 2.0 */
bool StubGenerator::generateApiCallStub( const FuncDescription& fd, ostream& f, const char* tr, const char* jt, const char* uct )
{
    int i, j, nBuffers = 0;
    int nParams = fd.countParams();
    char paramName[20][64]; // parameter with selector chosen. Example: "param[1].uv32bit"
    char paramSelector[20][16]; // Just selector

    bool unknownParam[20]; // params which are unkwown
    memset(unknownParam, 0, sizeof(unknownParam));

    ParamDescription* pd;
    ParamDescription::PD_TYPE type; // canonical type
    bool skipCall = false;

    /****************************
     * GENERATE FUNCTION HEADER *
     ****************************/
    f << "void ";
    f << "STUB_" << fd.getName() << "( TraceReader& " << tr << ", GLJumpTable& " << jt << ", GLJumpTable& " << uct << " )\n{\n";

    /********************************************************
     * GENERATE COMMENT CONTAINING ORIGINAL FUNCTION HEADER *
     ********************************************************/
    f << "    // " << fd.toString() << endl;


    /*****************************************
     * DETERMINE union PARAM SELECTOR & NAME *
     *****************************************/
    for ( i = 0; i < nParams; i++ )
    {
        pd = fd.getParam(i);
        type = pd->parseParamType();

        if ( pd->isPointer() || pd->isDoublePointer() )
        {
            if ( type != ParamDescription::UNKNOWN )
            {
                sprintf( paramName[i], "param[%d].ptr", i);
                strcpy(paramSelector[i], "ptr");
                nBuffers++;
            }
            else
                skipCall = true;
        }
        else
        {
            switch ( type )
            {
                case ParamDescription::BOOLEAN :
                    sprintf(paramName[i], "param[%d].boolean",i);
                    strcpy(paramSelector[i], "boolean");
                    break;
                case ParamDescription::BYTE:
                    sprintf( paramName[i], "param[%d].v8bit", i);
                    strcpy(paramSelector[i], "v8bit");
                    break;
                case ParamDescription::UBYTE:
                    sprintf( paramName[i], "param[%d].uv8bit", i);
                    strcpy(paramSelector[i], "uv8bit");
                    break;
                case ParamDescription::USHORT :
                    sprintf( paramName[i], "param[%d].uv16bit", i);
                    strcpy(paramSelector[i], "uv16bit");
                    break;
                case ParamDescription::SHORT :
                    sprintf( paramName[i], "param[%d].v16bit", i);
                    strcpy(paramSelector[i], "v16bit");
                    break;
                case ParamDescription::INT :
                case ParamDescription::SIZEI :
                    sprintf( paramName[i], "param[%d].v32bit", i);
                    strcpy(paramSelector[i], "v32bit");
                    break;
                case ParamDescription::ENUM :
                case ParamDescription::BITFIELD:
                case ParamDescription::UINT :
                    sprintf( paramName[i], "param[%d].uv32bit", i);
                    strcpy(paramSelector[i], "uv32bit");
                    break;
                case ParamDescription::FLOAT :
                case ParamDescription::CLAMPF :
                    sprintf( paramName[i], "param[%d].fv32bit", i);
                    strcpy(paramSelector[i], "fv32bit");
                    break;
                case ParamDescription::DOUBLE :
                case ParamDescription::CLAMPD :
                    sprintf( paramName[i], "param[%d].fv64bit", i);
                    strcpy(paramSelector[i], "fv64bit");
                    break;
                default:
                    if ( checkSpecialCall(fd.getName()) )
                    {
                        unknownParam[i] = true;
                        /*
                        char cast[256];
                        createCast(fd.getParam(i), cast);
                        */
                        sprintf( paramName[i], "param[%d].ptr", i);
                        strcpy( paramSelector[i], "ptr");
                    }
                    else
                        skipCall = true;
            }
        }
    }

    /********************************************************
     * SKIP CALL IF IT IS NOT PLAYABLE (PARAM TYPE UNKNOWN) *
     ********************************************************/
    if ( skipCall )
    {
        f << "    // Call not playable" << endl;
        f << "    " << tr << ".skipCall();" << endl;
        f << "}\n";
        return true;
    }

    /****************************
     * DECLARE LOCAL PARAMETERS *
     ****************************/
    f << "    maxParam = " << nParams << ";\n";
    /*
    if ( nParams > 0 ) // declare params
        f << "    Param param[" << nParams << "];" << endl;
        */
    for ( j = 0; j < nBuffers; j++ ) /* declare buffers */
        f << "    char buffer" << j << "[16*4096];" << endl; /* allocate buffers */
    j = 0;

    /****************************************
     * GENERATE CODE FOR READING PARAMETERS *
     ****************************************/
    for ( i = 0; i < nParams; i++ )
    {
        pd = fd.getParam(i);
        type = pd->parseParamType();
        if ( !pd->isPointer() && !pd->isDoublePointer() )
        {
            switch ( type )
            {
                case ParamDescription::UINT :
                case ParamDescription::ENUM :
                    //f << "    " << tr << ".";
                    //f << "readEnum(&" << paramName[i] << ");" << endl;
                    f << "    READ_ENUM_PARAM(" << i << ");" << endl;
                    break;
                case ParamDescription::BITFIELD: // can change in future ( with something like: readOring)
                    //f << "    " << tr << ".";
                    //f << "readOring(&" << paramName[i] << ");" << endl;
                    f << "    READ_ORING_PARAM(" << i << ");" << endl;
                    break;
                case ParamDescription::DOUBLE :
                case ParamDescription::CLAMPD :
                    //f << "    " << tr << ".";
                    //f << "readDouble(&" << paramName[i] << ");" << endl;
                    f << "    READ_DOUBLE_PARAM(" << i << ");" << endl;
                    break;
                case ParamDescription::BOOLEAN :
                    f << "    READ_BOOLEAN_PARAM(" << i << ");" << endl;
                    break;
                case ParamDescription::BYTE :
                case ParamDescription::UBYTE :
                case ParamDescription::SHORT :
                case ParamDescription::USHORT :
                case ParamDescription::INT :
                case ParamDescription::SIZEI :
                case ParamDescription::FLOAT :
                case ParamDescription::CLAMPF :
                    //f << "    " << tr << ".";
                    //f << "read(&" << paramName[i] << ");" << endl;
                    f << "    READ_PARAM(" << paramSelector[i] << "," << i << ");" << endl;
                    break;
                default:
                    f << "    // WARNING (read code coudn't be generated for this parameter)" << endl;
                    f << "    // This parameter must be set manually" << endl;
                    f << "    " << tr << ".";
                    f << "skipUnknown();" << endl;
            }
        }
        else
        {
            /* buffer ( 2 types: data buffer or small vector array (inlined) )
             * param[i].ptr contains the address of data buffer or 'buffer' parameter adress if it
             * is a small vector array ( contents stored in binary format in 'buffer' parameter )
             */
            if ( type != ParamDescription::UNKNOWN )
            {

                //f << "    " << tr << ".";
                //f << "readArray(&" << paramName[i] << ", buffer" << j <<", sizeof(buffer" << j << ")";
                f << "    READ_ARRAY_PARAM( " << i << ", buffer" << j;
                switch ( type )
                {
                    case ParamDescription::BOOLEAN :
                    case ParamDescription::BYTE :
                    case ParamDescription::UBYTE :
                        f << ", TraceReader::TEXT_BYTE";
                        break;
                    case ParamDescription::UINT :
                    case ParamDescription::ENUM :
                    case ParamDescription::INT :
                    case ParamDescription::SIZEI :
                    case ParamDescription::BITFIELD:
                        f << ", TraceReader::TEXT_INT";
                        break;
                    case ParamDescription::SHORT :
                    case ParamDescription::USHORT :
                        f << ", TraceReader::TEXT_SHORT";
                        break;
                    case ParamDescription::FLOAT :
                    case ParamDescription::CLAMPF :
                        f << ", TraceReader::TEXT_FLOAT";
                        break;
                    case ParamDescription::DOUBLE :
                    case ParamDescription::CLAMPD :
                        f << ", TraceReader::TEXT_DOUBLE";
                        break;
                    default:
                        f << ", TraceReader::TEXT_IGNORE";
                }
                f << ");" << endl;
                j++;
            }
            else
            {
                f << "    " << tr << ".";
                f << "skipUnknown();" << endl;
            }
        }
    }

    /**************************************************
     * GENERATE CODE FOR SKIPPING RESULT IF IT EXISTS *
     **************************************************/
    /* Note: Do not use skipCall() here. Not compatible with binary mode */
    if ( (pd = fd.getReturn()) != NULL )
    {
        f << "    " << tr << ".skipResult();" << endl;
    }
    f << "    " << tr << ".skipLine();" << endl;



    /******************************
     * Enabled only in debug mode *
     * DUMP call & parameters     *
     ******************************/

    /*
    f << "    #ifdef GLPLAYER_DEBUG" << endl;
    f << "    debugDump << \"";
    f << fd.getName() << "(\"";
    for ( i = 0; i < nParams; i++ )
    {
        if ( fd.getParam(i)->isPointer() || fd.getParam(i)->isDoublePointer() )
            f << " << (unsigned int)" << paramName[i];
        else
        {
            f << " << ";
            if ( unknownParam[i] )
            {
                f << "(unsigned int)";
            }
            f << paramName[i];
        }

        if ( i < nParams - 1 )
            f << " << \",\"";
    }
    f << " << \");\" << endl;" << endl;
    f << "    #endif" << endl;

    */

    char cast[256];

    /*******************************************
     * GENERATE CODE FOR CALLING USER CALLBACK *
     *******************************************/
    f << "    CHECK_USER_CALL(" << fd.getName() << ")\n";
    f << "        " << uct << "." << fd.getName() << "(";
    for ( i = 0; i < nParams; i++ )
    {
        if ( fd.getParam(i)->isPointer() || fd.getParam(i)->isDoublePointer() )
        {
            createCast(fd.getParam(i), cast);
            f << "(" << cast << ")";
        }
        else if ( unknownParam[i] )
        {
            createCast(fd.getParam(i), cast);
            f << "(" << cast << ")";
        }

        f << paramName[i];
        if ( i < nParams - 1 )
            f << ", ";
    }

    f << ");" << endl;

    /**************************************************
     * GENERATE CODE FOR CALLING REAL OPENGL FUNCTION *
     **************************************************/

    f << "    CHECK_GL_CALL(" << fd.getName() << ")" << endl;

    f << "        " << jt << "." << fd.getName() << "(";
    for ( i = 0; i < nParams; i++ )
    {
        if ( fd.getParam(i)->isPointer() || fd.getParam(i)->isDoublePointer() )
        {
            createCast(fd.getParam(i), cast);
            f << "(" << cast << ")";
        }
        else if ( unknownParam[i] )
        {
            createCast(fd.getParam(i), cast);
            f << "(" << cast << ")";
        }

        f << paramName[i];
        if ( i < nParams - 1 )
            f << ", ";
    }

    f << ");" << endl;
    f << "}" << endl;

    return true;

}


bool StubGenerator::generateAllApiCallStubs( const FuncExtractor& fe, ostream& f, const char* tr, const char* jt, const char* uct )
{
    int i;
    int nFuncs = fe.count();

    for ( i = 0; i < nFuncs; i++ )
    {
        generateApiCallStub(*fe.getFunction(i), f, tr, jt, uct);
        f << endl;
    }
    return true;
}


bool StubGenerator::generateAllApiCallStubsDeclarations( const FuncExtractor& fe, ofstream& f )
{
    int i;
    int nFuncs = fe.count();
    for ( i = 0; i < nFuncs; i++ )
    {
        f << "void ";
        f << "STUB_" << fe.getFunction(i)->getName() << "(TraceReader&, GLJumpTable&, GLJumpTable&);" << endl;
    }
    return true;
}

bool StubGenerator::generateCode()
{
    ofstream f;

    /********************************
     * Generate StubApiCalls.h file *
     ********************************/
    if ( openOFile(f, "StubApiCalls.h") )
    {

        writePreface(f);

        f << "#ifndef STUBAPICALLS_H\n    #define STUBAPICALLS_H\n\n"
             "#include \"TraceReader.h\"\n"
             "#include \"GLJumpTable.h\"\n"
             "#include \"GenericParam.h\"\n\n"

             "Param getLastCallParameter(unsigned int iParam);\n"
             "void setLastCallParameter(unsigned int iParam, const Param& p);\n\n";

        generateAllApiCallStubsDeclarations(fe, f);

        f << "\n#endif // STUBAPICALLS_H\n";
        f.close();
    }

    /**********************************
     * Generate StubApiCalls.cpp file *
     **********************************/
    if ( openOFile(f, "StubApiCalls.cpp") )
    {

        writePreface(f);

        f << "#include \"StubApiCalls.h\"\n\n";

        /*
        f << "#ifdef GLPLAYER_DEBUG\n";
        f << "extern ofstream debugDump;\n";
        f << "#endif\n\n";
        */

        /* create a macro for loading extension in windows */
        //f << "#ifdef WIN32" << endl;
        f << "#ifndef LOAD_JUMPTABLE_STATICALLY" << endl;
        f << "#define CHECK_GL_CALL(call)\\\n";
        f << "if ( JT.call == NULL ) {\\\n" <<
                "  ((unsigned long *)(&JT))[APICall_##call] = (unsigned long)JT.wglGetProcAddress(#call);\\\n" <<
                "  if ( JT.call == NULL )\\\n" <<
                "  {popup(#call,\"Call unsupported. Cannot be found in current gl implementation\");}\\\n" <<
                "}\\\n" <<
                "if ( JT.call != NULL )" << endl;
        f << "#else" << endl;
        f << "#define CHECK_GL_CALL(call)\\\n";
        f << "if ( JT.call == NULL )\\\n" <<
             "{ popup(#call,\"Call unsupported. Cannot be found in current gl implementation\"); }\\\n" <<
             "else" << endl;

        f << "#endif\n\n";

        f << "#define CHECK_USER_CALL(call) if ( UCT.call != NULL )\n\n";

        f << "#define READ_PARAM(unionSelector,whichParam)\\\n"
             "    if ( paramFlag & (1 << whichParam) )\\\n"
             "    {\\\n"
             "        Param dummy;\\\n"
             "        TR.read(&dummy.unionSelector);\\\n"
             "        paramFlag &= ~(1 << whichParam);\\\n"
             "    }\\\n"
             "    else\\\n"
             "        TR.read(&param[whichParam].unionSelector);\n\n";

        f << "#define READ_ENUM_PARAM(whichParam)\\\n"
             "    if ( paramFlag & (1 << whichParam) )\\\n"
             "    {\\\n"
             "        Param dummy;\\\n"
             "        TR.readEnum(&dummy.uv32bit);\\\n"
             "        paramFlag &= ~(1 << whichParam);\\\n"
             "    }\\\n"
             "    else\\\n"
             "        TR.readEnum(&param[whichParam].uv32bit);\n\n";

        f << "#define READ_ORING_PARAM(whichParam)\\\n"
             "    if ( paramFlag & (1 << whichParam) )\\\n"
             "    {\\\n"
             "        Param dummy;\\\n"
             "        TR.readOring(&dummy.uv32bit);\\\n"
             "        paramFlag &= ~(1 << whichParam);\\\n"
             "    }\\\n"
             "    else\\\n"
             "        TR.readOring(&param[whichParam].uv32bit);\n\n";


        f << "#define READ_DOUBLE_PARAM(whichParam)\\\n"
             "    if ( paramFlag & (1 << whichParam) )\\\n"
             "    {\\\n"
             "        Param dummy;\\\n"
             "        TR.readDouble(&dummy.fv64bit);\\\n"
             "        paramFlag &= ~(1 << whichParam);\\\n"
             "    }\\\n"
             "    else\\\n"
             "        TR.readDouble(&param[whichParam].fv64bit);\n\n";

        f << "#define READ_BOOLEAN_PARAM(whichParam)\\\n"
             "    if ( paramFlag & (1 << whichParam) )\\\n"
             "    {\\\n"
             "        Param dummy;\\\n"
             "        TR.readBoolean(&dummy.boolean);\\\n"
             "        paramFlag &= ~(1 << whichParam);\\\n"
             "    }\\\n"
             "    else\\\n"
             "        TR.readBoolean(&param[whichParam].boolean);\n\n";

        f << "#define READ_ARRAY_PARAM(whichParam, buf, mode)\\\n"
             "    if ( paramFlag & (1 << whichParam) )\\\n"
             "    {\\\n"
             "        Param dummy;\\\n"
             "        TR.readArray(&dummy.ptr, buf, sizeof(buf), mode);\\\n"
             "        paramFlag &= ~(1 << whichParam);\\\n"
             "    }\\\n"
             "    else\\\n"
             "        TR.readArray(&param[whichParam].ptr, buf, sizeof(buf), mode);\n\n";


        f << "static int paramFlag = 0;\n";
        f << "static Param param[32];\n";
        f << "static unsigned int maxParam = 0;\n\n";

        f << "Param getLastCallParameter(unsigned int iParam)\n"
             "{\n"
             "    if ( iParam >= maxParam )\n"
             "        panic(\"StubApiCalls\", \"getLastCallParameter\", \"The parameter does not exist\");\n"
             "    return param[iParam];\n"
             "}\n\n";

        f << "void setLastCallParameter(unsigned int iParam, const Param& p)\n"
             "{\n"
             "    // if ( iParam >= maxParam )\n"
             "    //    panic(\"StubApiCalls\", \"setLastCallParameter\", \"The parameter does not exist\");\n"
             "    param[iParam] = p;\n"
             "    paramFlag |= (1 << iParam);\n"
             "}\n\n";

        generateAllApiCallStubs(fe, f, "TR","JT", "UCT");
        f.close();
    }

    return true;

}


