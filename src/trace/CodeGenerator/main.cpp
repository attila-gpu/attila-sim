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

/* Include extractors interface */
#include "FuncExtractor.h"
#include "ConstExtractor.h"
#include "SpecificExtractor.h"
#include "support.h"
#include <cstdio>
#include "StringTokenizer.h"
/* Include generator interface */
#include "CodeGenerator.h"

/* Add generators definitions here */
#include "WrapperGenerator.h"
#include "GLIEntryPointsGenerator.h"
#include "StubGenerator.h"
#include "JTGenerator.h"
#include "GLNamesGenerator.h"
#include "SwitchBranchesGenerator.h"

#include <iostream> /* C++ i/o */
#include <vector>
#include <set>
#include <cstring>

using namespace std;


/**
 * Files from where data will be extracted
 */
const char* FILES[] =
{
    "gl.h",
    "glext.h",
    "mesa_wgl.h",
    "wglext.h",
    "GLIOthers.h"
};

/**
 *
 */
vector<string> inFiles;

/*
 * Code Generator objects
 */ 
vector<CodeGenerator *> generators;

char dirGLInterceptor[256] = ".";
char dirGLInstrument[256] = ".";
char dirTraceReader[256] = ".";
char dirGen[256] = ".";
char inGL[256] = ".";
char inGLConstants[256] = "NULL";
char inLibFuncsFile[256] = "gl.h";
char inSpecificFile[256] = "Specific.h";
char inSpecificStatsFile[256] = "SpecificStats.h";
char genTraceCode[256] = "NULL";

char* convertPath( char* path )
{
    char* start = path;
    
    if ( *path == '\0' )
        return start; /* empty path */

    while ( *path != '\0' )
    {
        #ifdef WIN32
            if ( *path == '/' )
                *path = '\\';
        #else
             if ( *path == '\\' )
             {
                *path = '/';
             }
        #endif
        path++;
    }
    path--;
    if ( *path == '\\' || *path == '/' )
        *path = '\0';

    return start;
}


void parseConfig( const char* config )
{

    ifstream f;
    f.open(config);
    if (!f.is_open() )
    {
        cout << "Config file not found... Using default values" << endl;
        /* no config file, use defaults */
        return ;
    }
    char option[4096];
    char* value;
    while ( !f.eof() )
    {
        f.getline(option,sizeof(option),'\n');
        value = option;
        while ( *value != '\0' && *value != '=' )
            value++;
        if ( *value == '=' )
        {
            *value = '\0';
            value++; /* skip '\0' */
            if ( EQ(option,"inGL") )
                strcpy(inGL, convertPath(value));
            else if ( EQ(option,"inGLConstants") )
                strcpy(inGLConstants,convertPath(value));
            else if ( EQ(option,"genTraceCode") )
                strcpy(genTraceCode,convertPath(value));
            else if ( EQ(option, "inputFiles") )
            {
                StringTokenizer st(value);
                char temp[256];
                if ( st.getToken(temp,sizeof(temp)) )
                {
                    inFiles.clear();
                    inFiles.push_back(string(temp));
                }

                while ( st.getToken(temp,sizeof(temp)) )
                    inFiles.push_back(string(temp));

                cout << "Input files detected: ";
                vector<string>::iterator it = inFiles.begin();
                for (; it != inFiles.end(); it++ )
                    cout << it->c_str() << " ";
                cout << endl;

            }
            else if ( EQ(option,"inLibFuncsFile") )
                strcpy(inLibFuncsFile, convertPath(value));
            else if ( EQ(option, "inSpecificFile") )
                strcpy(inSpecificFile, convertPath(value));
            else if ( EQ(option, "inSpecificStatsFile") )
                strcpy(inSpecificStatsFile,convertPath(value));
            else if ( EQ(option,"outGLInterceptor") )
                strcpy(dirGLInterceptor, convertPath(value));
            else if ( EQ(option, "outGLInstrument") )
                strcpy(dirGLInstrument, convertPath(value));
            else if ( EQ(option,"outTraceReader") )
                strcpy(dirTraceReader, convertPath(value));
            else if ( EQ(option,"outGen") )
                strcpy(dirGen, convertPath(value));
        }
    }    

    if ( EQ(inGLConstants,"NULL") || EQ(inGLConstants,"null") )
        strcpy(inGLConstants,inGL);
}

int main( int argc, char* argv[] )
{
    unsigned int i;
    unsigned int j = 0;
    
    /* default files */
    for ( i = 0; i < sizeof(FILES)/sizeof(FILES[0]); i++ )
        inFiles.push_back(string(FILES[i]));
    
    if ( argc < 2 )
        parseConfig("CGconfig.ini");        
    else
        parseConfig(argv[1]);

    //  Check if files already exist
    ifstream checkFile;
    bool allOutFilesCreated = true;
    string filename;

#ifdef WIN32

#define CHECK_FILE(dir, name) \
    filename = (dir); \
    filename.append(name); \
    checkFile.open(filename.c_str()); \
    allOutFilesCreated = allOutFilesCreated && checkFile.is_open(); \
    filename.clear();

    CHECK_FILE(dirGLInterceptor, "\\GLWrapper.cpp")
    CHECK_FILE(dirGLInterceptor, "\\GLWrapper.h")
    CHECK_FILE(dirGLInterceptor, "\\opengl3d.deff")

    CHECK_FILE(dirGLInstrument, "\\GLIEntryPoints.cpp")
    CHECK_FILE(dirGLInstrument, "\\GLIEntryPoints.h")

    CHECK_FILE(dirTraceReader, "\\StubApiCalls.cpp");
    CHECK_FILE(dirTraceReader, "\\StubApiCalls.cpp");

    CHECK_FILE(dirGen, "\\GenAPICall.gen")
    CHECK_FILE(dirGen, "\\GenGLConstantNames.gen")
    CHECK_FILE(dirGen, "\\GenGLFunctionNames.gen")
    CHECK_FILE(dirGen, "\\GenGLJumpTableFields.gen")
    CHECK_FILE(dirGen, "\\GenGLJumpTableSL.gen")
    CHECK_FILE(dirGen, "\\GenJumpTableWrapper.gen")
    CHECK_FILE(dirGen, "\\GenPlayableCalls.gen")
    CHECK_FILE(dirGen, "\\GenSwitchBranches.gen")

#undef CHECK_FILE

    if (allOutFilesCreated)
    {
        cout << "All autogenerated files already exist.  Skipping autogeneration" << endl;
        exit(0);
    }
#endif

    set<string> traceFunctions;

    if ( !EQ(genTraceCode,"NULL") && !EQ(genTraceCode,"null") )
    {
        FuncExtractor aux;
        j = aux.extractFunctions(genTraceCode);
        for ( i = 0; i < j; i++ )
            traceFunctions.insert(aux.getFunction(i)->getName());
    }

    cout << "Input files/directories: " << endl;
    if ( j > 0 )
    {
        cout << " J = " << j << endl;
        cout << "   WARNING: " << "Only generate trace code for the functions in this file: " << genTraceCode << endl;
        cout << "   Functions detected: " << j << endl;
        cout << "   The other functions will be just executed without tracing them" << endl;
        
    }
    cout << "  GL .h files directory set to: " << inGL << endl;
    cout << "  OGL Library function file: " << inLibFuncsFile << endl;
    cout << "  Specific file will be: " << inSpecificFile << endl;
    cout << "Output directories: " << endl;
    cout << "  To place GLInterceptor autogenerated files: " << dirGLInterceptor << endl;
    cout << "  To place TraceReader autogenerated files: " << dirTraceReader << endl;
    cout << "  To place Gen autogenerated files: " << dirGen << endl<< endl;

    /* extractor for GL Functions */
    FuncExtractor fe;

    /* extractor for GL functions supported by bGPU OGL library */
    FuncExtractor libFuncs;

    /* extractor for GL constants*/
    ConstExtractor ce;

    /* Extractor for functions with Specific behaviour */
    SpecificExtractor se;

    /* Extractor for advanced/specific stats */
    FuncExtractor feStats;


    /********************************************************
     * Create code generators. Add here new code generators *
     ********************************************************/

    if ( !EQ(dirGLInterceptor,"null") && !EQ(dirGLInterceptor,"NULL") )
    {
        if ( j <= 0 )
            generators.push_back( new WrapperGenerator(fe, se, feStats, dirGLInterceptor, dirGen ) );
        else
            generators.push_back( new WrapperGenerator(fe, se, feStats, dirGLInterceptor, dirGen, traceFunctions, true) );
    }

    if ( !EQ(dirGLInstrument, "null") && !EQ(dirGLInstrument, "NULL") )
        generators.push_back( new GLIEntryPointsGenerator(fe, dirGLInstrument) );
    if ( !EQ(dirTraceReader,"null") && !EQ(dirTraceReader,"NULL") )
        generators.push_back( new StubGenerator(fe, dirTraceReader) );
    if ( !EQ(dirGen,"null") && !EQ(dirGen,"NULL") )
        generators.push_back( new JTGenerator(fe, libFuncs, dirGen,"_JT") );
    
    char *pathGLI;
    char *pathGen;
    pathGen = ( EQ(dirGen,"null") || EQ(dirGen,"NULL") ? NULL : dirGen );
    pathGLI = ( EQ(dirGLInterceptor,"null") || EQ(dirGLInterceptor,"NULL") ? NULL : dirGLInterceptor );

    if ( pathGen != NULL ||pathGLI != NULL )
        generators.push_back( new GLNamesGenerator(fe, ce, pathGen, pathGLI) );
    
    
    if ( !EQ(dirGen,"null") && !EQ(dirGen,"NULL") )
        generators.push_back( new SwitchBranchesGenerator(fe, "tr", "jt", "uct", dirGen) );

    /*****************************
     * Load data into extractors *
     *****************************/
    

    char *filters[] = 
    {
        "*MESA",
        "*SUN",
        "*IBM",
        "ChoosePixelFormat",
        "DescribePixelFormat",
        "GetPixelFormat",
        "SetPixelFormat",
        "SwapBuffers"
    };

    for ( i = 0; i < sizeof(filters)/sizeof(filters[0]); i++ )
        fe.addFilter(filters[i]);
        
    char filePath[256];

    vector<string>::iterator it = inFiles.begin();

    for ( ; it != inFiles.end(); it++ )
    {
        strcpy(filePath, inGL);
#ifdef WIN32
        strcat(filePath,"\\");
#else
    strcat(filePath,"/");
#endif
        strcat(filePath,it->c_str());

        cout << "Extracting function definitions from: "  << filePath <<  endl;
        cout << "  Functions extracted: " << fe.extractFunctions( filePath ) << endl;
    }       

    for ( i = 0; i < sizeof(FILES)/sizeof(FILES[0]); i++ )
    {
        strcpy(filePath, inGLConstants);
#ifdef WIN32
        strcat(filePath,"\\");
#else
        strcat(filePath,"/");
#endif
        strcat(filePath,FILES[i]);

        cout << "Extracting constants definitions from: " << filePath << endl;
        cout << "   Constants extracted: " << ce.extractConstants(filePath) << endl;
    }

    
    cout << "Total functions extracted: " << fe.count() << endl;
    cout << "Total constants extracted: " << ce.count() << endl;

    cout << "Specific Functions extracted: " << se.extract(inSpecificFile) << endl << endl;

    cout << "Functions implementing stat code: " << feStats.extractFunctions(inSpecificStatsFile) << endl << endl;
    
    int nLibFuncs = libFuncs.extractFunctions(inLibFuncsFile);

    cout << "Functions from bGPU OGL implementation: " << nLibFuncs << endl;

    cout << "Generating code..." << endl;
    
    /*****************
     * Generate code *
     *****************/
    for ( i = 0; i < generators.size(); i++ )
    {          
        generators[i]->dumpInfo();
        generators[i]->generateCode();
        cout << " OK" << endl;
    }

    cout << "Finished!" << endl;
   
    return 0;
}
