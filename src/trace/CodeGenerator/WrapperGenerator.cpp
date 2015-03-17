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

#include "WrapperGenerator.h"
#include "NameModifiers.h"
#include "support.h"
#include <iostream>
#include <cstdio>
#include <cstring>

//#define DEBUG_GLI

using namespace std;


WrapperGenerator::WrapperGenerator( const FuncExtractor& fe, const SpecificExtractor& se,
                                    const FuncExtractor& feStats,const char* oDirWrapper,
                                    const char* oDirGen )
 : FuncGenerator(), fe(fe), se(se), feStats(feStats), candidates(0), genTraceCode(false)
{
    strcpy(this->oDirGen, oDirGen);
    setOutputDir(oDirWrapper);
}

WrapperGenerator::WrapperGenerator( const FuncExtractor& fe, const SpecificExtractor& se,
                                    const FuncExtractor& feStats,const char* oDirWrapper,
                                    const char* oDirGen, set<string>& candidates, 
                                    bool genTraceCode )
 : FuncGenerator(), fe(fe), se(se), feStats(feStats), 
   candidates(&candidates), genTraceCode(genTraceCode)
{
    strcpy(this->oDirGen, oDirGen);
    setOutputDir(oDirWrapper);
}


void WrapperGenerator::dumpInfo() const
{
    cout << "WrapperGenerator generates:\n  { GLWrapper.h, GLWrapper.cpp, PlayableCalls.gen }";
}


bool WrapperGenerator::generateWrapperCall( std::ostream& fout, const char* writerName, 
                                            const char* jumpTableName, const FuncDescription& fd, 
                                            const SpecificItem* sfd, bool hasSpecificStat )
{
    bool unplayableFlag = false;

    int i;
        
    int iParam = 0; // artificial parameter counting

    char paramName[20][64];    

    NameModifiers md( fd.getName() ); // extract function name modifiers

    /*
     * Parameter which is the only pointer in the header function and the function name
     * has any modifier. It is assumed that modifiers refers to this parameter
     *
     * Possible values:
     *
     *  > 0 : the parameter with value ptrParam is the only which is a pointer and there are modifiers
     *        in function name ( are assumed to be associated with this parameter )
     *
     *  -1  : there are no parameters which are pointers or there are no modifiers in the function
     *
     *  -2  : more than 1 parameter being a pointer
     *
     */
    int ptrParam = -1; 

    ParamDescription* pd;
    
    /****************************
     * GENERATE FUNCTION HEADER *
     ****************************/
    if ( fd.getReturn() != NULL )
        fout << fd.getReturn()->toString() << " ";
    else
        fout << "void ";

    fout << "GLAPIENTRY ";
    
    fout << fd.getName() << "(";

    iParam = fd.countParams();
    
    /************************************************
     * GENERATE AND EXAMINE PARAMETER NAMES & TYPES *
     ************************************************/
    for ( i = 0; i < iParam; i++ )
    {
        pd = fd.getParam(i);
        
        if ( pd->isConst() )
            fout << "const ";
        
        fout << pd->getType() << " ";
        
        if ( pd->isPointer() || pd->isDoublePointer() ) 
        {   
            if ( md.isVector() && ptrParam == -1 )
                ptrParam = i;
            else if ( ptrParam >= 0 )
                ptrParam = -2;

            if ( pd->isPointer() )
                fout << "*";
            else // double pointer
                fout << "**";
        }
        
        if ( pd->getName() == NULL ) // create names for variables with type but without name
            sprintf(paramName[i],"_p%d",i);
        else
            strcpy(paramName[i],pd->getName());            

        fout << paramName[i];
        
        if ( i < iParam - 1 )
            fout << ", ";
    }

    fout << ")\n{" << endl;         


    /**************************
     * GENERATE FUNCTION CODE *
     **************************/

    bool skipGenCode = ( sfd != NULL && sfd->getOCSFlag() );

    if ( candidates != 0 )
    {
        set<string>::iterator it = candidates->find(string(fd.getName()));
        if ( it == candidates->end() )
            skipGenCode = true; 
    }
     
     // declare a result var if the function returns a value

    ParamDescription* ret = NULL;    

    ret = fd.getReturn();
    
    if ( !skipGenCode && ret != NULL ) 
    {
        fout << "    ";
        if ( ret->isConst() )
           fout << "const ";
        fout << ret->getType() << " ";
        if ( ret->isPointer() )
            fout << "*";
        else if ( ret->isDoublePointer() )
            fout << "**";        
        fout << "_result;" << endl;
    }
/*
    fout << "    " << writerName << ".writeAPICall(APICall_" << fd.getName() << ");" << endl;
    fout << "    " << writerName << ".flush();" << endl;
    */

    //fout << "#ifndef WRAP_ONLY_MODE" << endl;




    /**************************
     * INIT TRACE IF REQUIRED *
     **************************/
    /* it has effect only the first time called */
    fout << "    GLInterceptor::init();" << endl;
    fout << "    GLInterceptor::startTWCallMode(APICall_" << fd.getName() << ");\n";    


    /**
     * Print call name first of all (allows to debug which call causes an error)
     * But the generated trace may be not executable due to some calls (wgl's),
     * call anothers and printing first the function name generates a corrupted trace
     */
//#define DEBUG_GLI
#ifdef DEBUG_GLI
    fout << "    " << writerName << ".writeAPICall(APICall_" << fd.getName() << ");" << endl;
    fout << "    " << writerName << ".flush();" << endl;
#endif



    /*****************************************
     * GENERATE CODE FOR SPECIFIC STATISTICS *
     *****************************************/
    if ( hasSpecificStat )
    {
        fout << "    // Specific stats code" << endl;
        fout << "    ";
        generateFuncCall(fout, fd.getName(), "_STAT", paramName, fd.countParams());
        fout << ";" << endl;
    }    

    /****************************************************
     * BASIC CODE FOR COMMON STATISTICS ( count calls ) *
     ****************************************************/
    // counting is the first operation done ( before call specific or log any parameter )
    fout << "    _COUNT(APICall_" << fd.getName() << ");" << endl;

    //fout << "#endif" << endl;


     // check if specific call is required before any logging
    if ( sfd != NULL && sfd->isSpecificCallRequired(0) && !skipGenCode )
    { 
        //fout << "#if !defined(STATS_ONLY_MODE) && !defined(WRAP_ONLY_MODE)" << endl;
        // call specific
        fout << "    // [GEN-CODE info: Specific user call]" << endl;
        fout << "    ";
        // generate specific call
        generateFuncCall(fout,fd.getName(),"_SPECIFIC",paramName,fd.countParams());        
        fout << ";" << endl;
        //fout << "#endif" << endl;
    }

    /********************************************
     * Generate code for suporting hacked calls *
     ********************************************/
    // Note: Mandatory after Specific call before any logging ( if exist this specific call )
    if (sfd == NULL || (!sfd->getOCSFlag() && !sfd->getReplaceFlag()))
        fout << "    CHECKCALL(" << fd.getName() << ");" << endl;

    // APICall name is traced before calling real call (allow to guess which functions have fail)

    if ( sfd != NULL && ( sfd->getReplaceFlag() || sfd->getOCSFlag()) ) 
        fout << "    // Calling SPECIFIC function instead orginal (assumed that SPECIFIC will call original)\n";
    
    // generate code for calling real OpenGL function
    if ( ret != NULL && !skipGenCode ) 
        fout << "    _result = ";
    else if ( ret != NULL && skipGenCode )
        fout << "    return "; // return directly
    else
        fout << "    ";
    

    if ( sfd != NULL && (sfd->getReplaceFlag() || sfd->getOCSFlag()) ) 
    {
        // use SPECIFIC instead of real call
        //fout << sfd->getFunction()->getName() << "(";
        generateFuncCall(fout, sfd->getFunction()->getName(), NULL, paramName, 
                         sfd->getFunction()->countParams());
        fout << ";";
    }

    else 
    {
        fout << "DO_REAL_CALL(";
        generateFuncCall(fout, fd.getName(), NULL, paramName, fd.countParams());
        fout << ");";
    }

    fout << endl;

    /*
     * Write APICall name
     */
#ifndef DEBUG_GLI
    fout << "    " << writerName << ".writeAPICall(APICall_" << fd.getName() << ");" << endl;
#endif
    fout << "    " << writerName << ".writeMark(\"(\");" << endl;

    if ( skipGenCode ) {
        fout << "}" << endl;
        return true; /* OCS Flag, assumes playable */
    }

    //fout << "#if !defined(STATS_ONLY_MODE) && !defined(WRAP_ONLY_MODE)" << endl;
    
    
    
    /******************
     * LOG PARAMETERS *
     ******************/
    for ( i = 0; i < iParam; i++ )
    {           
        /**
         * Log parameter only if is not disabled
         */
        if ( sfd == NULL || !sfd->getParam(i+1) ) // i+1 because params goes from 1 to N
        {            
            // get current parameter
            pd = fd.getParam(i); 
            if ( i == ptrParam ) // vector parameter ( parameters can be deducted from function name )
            {                
                if ( pd->isDoublePointer() ) { // double pointer
                    fout << "    " << writerName << ".writeUnknown(&" << paramName[i] << ", " <<
                        md.countParams() << ");" << endl;
                    unplayableFlag = true;
                }
                else // simple pointer ( common vector )
                { 
                    if ( EQ(pd->getType(),"GLenum") )
                        fout << "    " << writerName << ".writeEnum(" << paramName[i] << ", " << 
                            md.countParams() << ");" << endl;
                    else
                        fout << "    " << writerName << ".write(" << paramName[i] << ", " << 
                            md.countParams() << ");" << endl;
                }
            }
            else // no deducible vector parameter or common parameter
            {                
                if (EQ(pd->getType(),"GLenum") && !pd->isPointer() && !pd->isDoublePointer())
                    fout << "    " << writerName << ".writeEnum(" << paramName[i] << ");" << endl;
                else if (EQ(pd->getType(),"GLbitfield") && !pd->isPointer() && !pd->isDoublePointer())
                    fout << "    " << writerName << ".writeOring(" << paramName[i] << ");" << endl;
                else 
                {
                    if ( isKnownType(pd->getType()) && ( /*md.isVector() ||*/
                        (!pd->isPointer() && !pd->isDoublePointer()) ))
                    {
                        fout << "    " << writerName << ".write(" << paramName[i] << ");" << endl;
                    }
                    else  {
                        fout << "    " << writerName << ".writeUnknown(&" << paramName[i] << ");" << endl;
                        unplayableFlag = true;
                    }
                }
            }
        
            if (i < iParam - 1)
                fout << "    " << writerName << ".writeMark(\",\");" << endl;
        }
        
        // check if specific call is requested after the param is logged
        if ( sfd != NULL && sfd->isSpecificCallRequired(i+1) )
        { 
            // call specific
            fout << "    // [GEN-CODE info: Specific user call]" << endl;
            fout << "    ";
            generateFuncCall(fout, fd.getName(), "_SPECIFIC", paramName, fd.countParams());
            fout << ";" << endl;
        }
    }

    fout << "    " << writerName << ".writeMark(\")\");" << endl;

    // write result
    if ( ret != NULL ) 
    {        
        fout << "    " << writerName << ".writeMark(\"=\");" << endl;
        if ( sfd == NULL || sfd->logReturn() ) {
            fout << "    if ( GLInterceptor::isReturnTraced() )\n    ";
            if (EQ(ret->getType(),"GLenum"))
            {
                
                fout << "    " << writerName << ".writeResultEnum(_result);" << endl;
                
            }
            else {
                if ( isKnownType(ret->getType()) && !ret->isPointer() && !ret->isDoublePointer() )
                {
                    fout << "    " << writerName << ".writeResult(_result);" << endl;
                }
                else 
                {
                    fout << "    " << writerName << ".writeResultUnknown(&_result);" << endl;
                    unplayableFlag = true;
                }
            }
            fout << "    else\n        " << writerName << ".writeResult(\"NOT TRACED\");\n";
        }
        // Important note. If return is not logged return must be logged in specific call
        if ( sfd != NULL && sfd->isSpecificCallAfterReturnRequired() )
        {
            fout << "    // [GEN-CODE info: Specific user call after return log]" << endl;
            if ( !sfd->logReturn() )
                fout << "    // [Note: This specific call must log return]" << endl;
            fout << "    ";
            generateFuncCall(fout, fd.getName(),"_SPECIFIC",paramName, fd.countParams());
            fout << ";" << endl;
        }
        
        fout << "    " << writerName << ".writeMark(\"\\n\");" << endl;
        fout << "    GLInterceptor::restoreTWMode(APICall_" << fd.getName() << ");\n";
        //fout << "#endif" << endl;
        //fout << "    " << writerName << ".flush();" << endl;
        fout << "    return _result;" << endl;
    }
    else
    {
        fout << "    " << writerName << ".writeMark(\"\\n\");" << endl;
        fout << "    GLInterceptor::restoreTWMode(APICall_" << fd.getName() << ");\n";
        //fout << "#endif" << endl;
        //fout << "    " << writerName << ".flush();" << endl;
    }
    

    fout << "}" << endl;

    return !unplayableFlag;
    
}


bool WrapperGenerator::generateAllWrapperCalls( ostream& f, const char* writerName, 
                              const char* jumpTableName, const FuncExtractor& fe )
{
    int i, nFuncs;

    nFuncs = fe.count();

    char funcName[256];

    isCallPlayable = new bool[nFuncs];

    
    for ( i = 0; i < nFuncs; i++ )
    {
        strcpy(funcName, fe.getFunction(i)->getName());
        SpecificItem* si = se.getSpecificFunction(funcName);
        strcat(funcName, "_STAT");
        bool hasStat = ( feStats.getFunction(funcName) ? true : false );
        isCallPlayable[i] = generateWrapperCall(f, writerName, jumpTableName, *fe.getFunction(i), si, hasStat);
        f << endl;
    }

    countUnplayables = 0;

    return true;
}

bool WrapperGenerator::generateCode()
{
    ofstream f;



    if ( openOFile(f,"GLWrapper.h") )
    {

        writePreface(f);

        f << "#ifndef GLWRAPPER_H\n";
        f << "    #define GLWRAPPER_H\n\n";
    
        f << "#include \"GLInterceptor.h\"\n";
        f << "#include \"wglext.h\"\n\n";
        f << "#define WGLAPI GLAPI\n\n";

        generateFuncDeclarations(f, fe);
        //generateAllWrapperDeclarations(f, fe);

        f << "\n#endif // GLWRAPPER_H\n";

        f.close();
    }

    /*******************************
     * Generate GLWrapper.cpp file *
     *******************************/
    if ( openOFile(f, "GLWrapper.cpp") )
    {
        writePreface(f);

        f << "#include \"glWrapper.h\"\n";
        f << "#include \"Specific.h\"\n";
        f << "#include \"SpecificStats.h\"\n";
        f << "#include <cstdlib> // for support exit() call\n\n";

        f << "#define _W GLInterceptor::tw\n";
        f << "#define _JT GLInterceptor::jt\n";
        f << "#define _COUNT(call) GLInterceptor::statManager.incCall(call)\n";
        f << "#define _ISHACK GLInterceptor::isHackMode()\n";
        
        f << "#define RESOLVE_CALL(call) ((unsigned long *)(&_JT))[APICall_##call] = (unsigned long)_JT.wglGetProcAddress(#call)\n";
        f << "#define CHECKCALL(call) if (_JT.##call == NULL) { RESOLVE_CALL(call); if (_JT.##call == NULL) { panic(\"Wrapper.cpp\", \"Call cannot be resolved\", #call); }}\n\n";

        //f << "#define CHECKCALL(call) if ( _JT.##call == NULL ) { if (!_ISHACK) { panic(\"CHECKCALL\",\"Call unsupported (NULL)\",#call); exit(0); } else popup(\"Call HACKED ;-)\",#call);} else\n\n";
    
        generateAllWrapperCalls(f, "_W", "_JT", fe );
    
        f.close();
    }

    
    /* note: Mandatory after generate all wrapper calls */
    setOutputDir(oDirGen);
    if ( openOFile(f,"PlayableCalls.gen") )
    {
        writePreface(f);

        f << "#ifndef PLAYABLECALLS_GEN\n";
        f << "    #define PLAYABLECALLS_GEN\n\n";

        int nFuncs = fe.count();
        int nYes = 0;
        int nNo = 0;
        for ( int i = 0; i < nFuncs; i++ )
        {
            if ( isCallPlayable[i] )
            {
                f << "true";
                nYes++;
            }
            else
            {
                f << "false";
                nNo++;
            }
            if ( i < nFuncs - 1 )
                f << ",";
            f << endl;
        }

        f << "\n// Total calls: " << nFuncs << endl;
        f << "// # playables: " << nYes << endl;
        f << "// # unplayables: " << nNo << endl;

        delete[] isCallPlayable;

        f << "\n#endif // PLAYABLECALLS_GEN\n";

        f.close();
    }

    return true;
}

