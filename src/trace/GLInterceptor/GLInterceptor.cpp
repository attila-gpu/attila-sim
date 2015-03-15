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

/* GLInterceptor.cpp : Defines the entry point for the DLL application */

#include "GLInterceptor.h"
#include "GLWrapper.h"
#include <ostream>
#include <cstdio>
#include <cstdlib>
#include <support.h>
#include "Stats.h"
#include <vector>
#include <sys/timeb.h>
#include <time.h>
#include "ShProgramInfo.h"
#include "UserStats.h"
#include <fstream>
#include "GLIVersion.h"
#include "includelog.h"
#include <sstream>

using includelog::logfile; // make logfile() visible
using std::stringstream;

using namespace std;

//bool GLInterceptor::traceCall[APICall_UNDECLARED]; // -> false = yes, true -> no
bool GLInterceptor::isStateCall[APICall_UNDECLARED];
bool GLInterceptor::initCalled = false;

// Default values
int GLInterceptor::resWidth = 800; 
int GLInterceptor::resHeight = 600;

bool GLInterceptor::compressedOutput = false;

BufferManager GLInterceptor::bm;

TraceWriter::TW_MODE GLInterceptor::initialTWMode = TraceWriter::devNull;

void GLInterceptor::startTWCallMode(APICall apicall)
{
    if ( isStateCall[apicall] ) // state call, must be traced always
    {
        if ( currentFrame > lastFrame && lastFrame != -1 )
        {
            static bool f = true;
            if ( f )
            {
                tw.writeMark("\n");
                tw.writeComment("Stop tracing...");
                f = false;
            }
            tw.setMode(tw.devNull);
        }
        return ; 
    }

    if ( currentFrame < firstFrame  )
        tw.setMode(tw.devNull); // disable tracing
    else if ( currentFrame > lastFrame && lastFrame != -1 )
        tw.setMode(tw.devNull); // disable tracing
}

void GLInterceptor::restoreTWMode(APICall apicall)
{
    static TraceWriter::TW_MODE prevM = GLInterceptor::getInitialTWMode();
    tw.setMode(prevM);
}

void GLInterceptor::initStateCallsArray()
{
    for ( int i = 0; i < APICall_UNDECLARED; i++ )
        isStateCall[i] = true;

#define SET_AS_NOTSTATE_CALL(apicall) isStateCall[APICall_##apicall] = false

    /*
    SET_AS_NOTSTATE_CALL(glNormal3f);
    SET_AS_NOTSTATE_CALL(glVertex4f);
    SET_AS_NOTSTATE_CALL(glVertex3f);
    SET_AS_NOTSTATE_CALL(glVertex2f);
    SET_AS_NOTSTATE_CALL(glVertex4fv);
    SET_AS_NOTSTATE_CALL(glVertex3fv);
    SET_AS_NOTSTATE_CALL(glVertex2fv);
    SET_AS_NOTSTATE_CALL(glBegin);
    SET_AS_NOTSTATE_CALL(glEnd);
    SET_AS_NOTSTATE_CALL(glDrawElements);
    SET_AS_NOTSTATE_CALL(glDrawArrays);
    */
    //SETCALL(wglSwapBuffers);

#undef SET_AS_NOTSTATE_CALL

}


/* Flags with defaults */

bool GLInterceptor::GLILogFlags[4] = { true, false, true, true };

char GLInterceptor::outputTraceFile[256] = "__tracefile.txt";
char GLInterceptor::statsPerFrameFile[256] = "__statsPF.txt";
char GLInterceptor::statsTotalFile[256] = "__statsT.txt";
char GLInterceptor::statsPerBatchFile[256] = "__statsPB.txt";

int GLInterceptor::firstFrame = 1; // first one
int GLInterceptor::lastFrame = -1; // infinite
int GLInterceptor::currentFrame = 0;

bool GLInterceptor::hackMode = false;
bool GLInterceptor::executeCalls = true;
int    GLInterceptor::splitLevel = 0; /* no split */
int GLInterceptor::cacheSz = 100;
bool GLInterceptor::acceptDeferredBuffers = false;
bool GLInterceptor::vertical = false; /* by default -> dump horizontal */
bool GLInterceptor::traceReturn = true;

/* Static components */
TraceWriter GLInterceptor::tw;
GLJumpTable GLInterceptor::jt;
GLIStatsManager GLInterceptor::statManager;


GLJumpTable GLInterceptor::jtWrapper =
{
    #include "JumpTableWrapper.gen"
};


//int GLInterceptor::callCounter[2][APICall_UNDECLARED];
//int GLInterceptor::pos = 0;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{        

    const char* tracefile;
    const char* statsPerFrameFile;
    const char* statsPerBatchFile;
    const char* statsTotalFile;

    bool check;
    
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:    
            logfile().enable(); // activate log object
            logfile().pushInfo(__FILE__, __FUNCTION__);            
            
            if ( !logfile().config("GLILog.ini") )
            {
                // .ini file not found (use defaults)
                logfile().open("GLILog.txt");                
                logfile().logFile(true);
                logfile().logFunction(true);
                logfile().enableLevels(includelog::Init | includelog::Panic);
            }
            logfile().writeConfig();
            logfile().write(includelog::Init, "DLL_PROCESS_ATTATCH\n");
            
            /*
             * Warning: ( Do not put any code here )
             *
             * @see: http://weblogs.asp.net/oldnewthing/archive/2004/01/27/63401.aspx
             */
                        
            break;
        case DLL_PROCESS_DETACH:

            logfile().pushInfo(__FILE__, __FUNCTION__);
            logfile().write(includelog::Init, "DLL_PROCESS_DETACH\n");
            
            // log buffers
            tracefile = GLInterceptor::getOutputFile(GLInterceptor::FILE_TRACEFILE);
            statsPerFrameFile = GLInterceptor::getOutputFile(GLInterceptor::FILE_STATS_PER_FRAME);
            statsTotalFile = GLInterceptor::getOutputFile(GLInterceptor::FILE_STATS_TOTAL);
            statsPerBatchFile = GLInterceptor::getOutputFile(GLInterceptor::FILE_STATS_PER_BATCH);

            if ( GLInterceptor::isLogFlagEnabled(GLInterceptor::LOG_TRACE) )
            {
                logfile().write(includelog::Init, "Flushing Trace Writer object... ");
                GLInterceptor::tw.flush();
                logfile().write(includelog::Init, "OK\n", false);
                if ( GLInterceptor::isLogFlagEnabled(GLInterceptor::LOG_BUFFERS) )
                {                    
                    stringstream ss;
                    ss << "Saving " << GLInterceptor::getBufferManager().count() 
                       << " Buffer descriptors";
                    uint deferredBufs = GLInterceptor::getBufferManager().countDeferredBuffers();
                    if ( deferredBufs != 0 )
                        ss << " (" << deferredBufs << " deferred buffers)... ";
                    else
                        ss << "... ";
                    logfile().write(includelog::Init, ss.str());
                    GLInterceptor::getBufferManager().close();
                    logfile().write(includelog::Init, "OK\n", false);
                }
                else
                    logfile().write(includelog::Init, "Saving buffers IGNORED - LOG_BUFFERS is disabled\n");
            }

            logfile().write(includelog::Init, "Saving shader statistics... ");
            if ( ShProgramManager::instance().writeInfo("ShaderStats.csv") )
                logfile().write(includelog::Init, "OK\n", false);
            else logfile().write(includelog::Init, "FAIL\n", false);

            logfile().write(includelog::Init, "Saving batch statistics... "); 
            // GLInterceptor::statManager.dumpBatchStats(statsPerBatchFile);
            logfile().write(includelog::Init, " [IGNORED by this GLI version]\n", false);

            logfile().write(includelog::Init, "Saving frame statistics... ");
            check = GLInterceptor::statManager.dumpFrameStats(statsPerFrameFile, 
                      GLInterceptor::isVerticalDumpMode(), GLInterceptor::getSplitLevel());
            if ( check ) { logfile().write(includelog::Init, "OK\n", false); }
            else { logfile().write(includelog::Init, "FAIL\n", false); }

            if ( GLInterceptor::isLogFlagEnabled(GLInterceptor::LOG_TRACE) )
            {
                if ( GLInterceptor::isLogFlagEnabled(GLInterceptor::LOG_BUFFERS) )
                    GLInterceptor::getBufferManager().close();
            }

            
            logfile().write(includelog::Init, "Trace finished\n");


            /*
                        
            if ( GLInterceptor::isLogFlagEnabled(GLInterceptor::LOG_STATS_PER_FRAME) )
                GLInterceptor::statManager.dumpCallsPerFrame(statsPerFrameFile);
            
            if ( GLInterceptor::isLogFlagEnabled(GLInterceptor::LOG_STATS_TOTAL) )
                GLInterceptor::statManager.dumpTotalCalls(statsTotalFile);        
            */

            break;

        case DLL_THREAD_ATTACH :
            logfile().pushInfo(__FILE__, __FUNCTION__);
            logfile().write(includelog::Init, "DLL_THREAD_ATTACH\n");
            break;

        case DLL_THREAD_DETACH :
            logfile().pushInfo(__FILE__, __FUNCTION__);
            logfile().write(includelog::Init, "DLL_THREAD_DETACH\n");
            break;

        default:
            
            {
                logfile().pushInfo(__FILE__, __FUNCTION__);
                stringstream ss;
                ss << "DllMain unknown ID - ul_reason_for_call value = " 
                    << ul_reason_for_call << "\n";
                logfile().write(includelog::Init, ss.str());
            }

            break;
            
    }

    logfile().popInfo();

    return TRUE;
}

int GLInterceptor::getSplitLevel()
{
    return splitLevel;
}

bool GLInterceptor::isVerticalDumpMode()
{
    return vertical;
}

void GLInterceptor::init(void)
{    
    if ( initCalled )
        return ;

    logfile().pushInfo(__FILE__,__FUNCTION__);

    setFrame(0);

    GLInterceptor::setReturnTraced(true); // default

    initStateCallsArray();

    /* Load user configuration */

    FILE* check = fopen("GLIconfig.ini", "r");
    if ( check == NULL )
    {
        check = fopen("config.ini", "r");
        if ( check != NULL )
        {
            fclose(check);
            parseConfigFile("config.ini");
        }
        else
            logfile().write(includelog::Init, "Configuration file  not found. Using default configuration\n");
    }
    else
    {
        fclose(check);
        parseConfigFile("GLIconfig.ini");
    }
    
    /* Start stats */

    logfile().write(includelog::Init, "Init Stats Manager... ");
    statManager.init(1);
    logfile().write(includelog::Init, "OK\n", false);

    /* Create user stats */
    logfile().write(includelog::Init, "Creating user statistics...\n");
    
    vcStat = new VertexStat("VERTEXES");
    statManager.addUserStat(vcStat);    
    logfile().write(includelog::Init, " \"VERTEXES\" statistic created.\n", false);
    
    tcStat = new TriangleStat("TRIANGLES");
    statManager.addUserStat(tcStat);
    logfile().write(includelog::Init, " \"TRIANGLES\" statistic created.\n", false);

    vshInstrCountStat = new InstructionCount("AVG. VSH INSTRS. PER VERTEX", 
                                             GL_VERTEX_PROGRAM_ARB,
                                             ShProgramManager::instance());
    statManager.addUserStat(vshInstrCountStat);
    logfile().write(includelog::Init, " \"AVG. VSH INSTRS. PER VERTEX\" statistic created.\n",false);
    
    fshInstrCountStat = new InstructionCount("AVG. FSH INSTRS. PER FRAGMENT", 
                                             GL_FRAGMENT_PROGRAM_ARB,
                                             ShProgramManager::instance());
    statManager.addUserStat(fshInstrCountStat);
    logfile().write(includelog::Init, " \"AVG. FSH INSTRS. PER FRAGMENT\" statistic created.\n", false);
    
    texInstrCountStat = new TextureLoadsCount("AVG. FSH TEXTURE INSTRS. PER FRAGMENT",
                                              ShProgramManager::instance());
    statManager.addUserStat(texInstrCountStat);
    logfile().write(includelog::Init, " \"AVG. FSH TEXTURE INSTRS. PER FRAGMENT\" statistic created.\n", false);

    fpAndAlphaStat = new FPAndAlphaStat("FP && Alpha enabled");
    //statManager.addUserStat(fpAndAlphaStat);
    //LOG(0, Log::log() << fpAndAlphaStat->getName() << " statistic created.\n";)

    logfile().write(includelog::Init, "User statistics created.\n", false);

    stringstream ss;
    if ( GLILogFlags[LOG_BUFFERS] )
    {
        ss.str("");
        ss << "BM -> acceptDeferredBuffers set to "
            << (acceptDeferredBuffers ? "TRUE" : "FALSE") << "\n";
        logfile().write(includelog::Init, ss.str());
        bm.acceptDeferredBuffers(acceptDeferredBuffers);

        logfile().write(includelog::Init, "BM -> Opening for storing BufferDescriptor.dat & MemoryRegions.dat... ");
        bm.open("BufferDescriptors.dat", "MemoryRegions.dat", BufferManager::Storing);
        logfile().write(includelog::Init, "OK\n", false);

        ss.str("");
        ss << "BM -> Cache size set to " << cacheSz << "\n";
        logfile().write(includelog::Init, ss.str());
        bm.setCacheMemorySize(cacheSz);
    }

    initCalled = true;    
    
    if ( GLILogFlags[LOG_TRACE] )
    {
        ss.str("");
        ss << "Creating output trace file '" << outputTraceFile << "' ";
        logfile().write(includelog::Init, ss.str());
        if ( tw.open( outputTraceFile, compressedOutput ))
            logfile().write(includelog::Init, "OK\n", false);
        else
        {
            logfile().write(includelog::Panic, "FAIL [exiting...]\n", false);
            panic("GLInterceptor", "init()", "Error creating output tracefile");
        }

        tw.writeTraceFormat();
        tw.writeComment("Trace generated with GLInterceptor 0.1");

        struct _timeb timebuffer;
        char *timeline;

        _ftime(&timebuffer);
        timeline = ctime(&(timebuffer.time));

        char theTime[256];

        sprintf(theTime, "%s", timeline);

        theTime[strlen(theTime)-1] = 0;
        
        tw.writeComment(theTime);
        

        //if ( firstFrame > 0 )
        //{
            initialTWMode = tw.getMode();
            tw.setMode(TraceWriter::devNull);
        //}
    }
    else // disable logging
    {
        logfile().write(includelog::Init, "Trace file set to NULL [no trace generated]\n");
        tw.setMode(TraceWriter::devNull);
    }

    char openGLPath[1024];
    
    (void)GetSystemDirectory(openGLPath,sizeof(openGLPath));
    
    strcat(openGLPath,"\\opengl32.dll");

    ss.str("");
    ss << "Loading real call table from " << openGLPath << "... ";
    logfile().write(includelog::Init, ss.str());    
        
    if ( loadGLJumpTable(jt,openGLPath) )
        logfile().write(includelog::Init, "OK\n", false);
    else
    {
        logfile().write(includelog::Panic, "FAIL [exiting...]\n", false);
        panic( "GLInterceptor", "init()", "Error loading call table");
    }


    logfile().write(includelog::Init, "Init completed\n");
    logfile().popInfo();
}


bool GLInterceptor::parseConfigFile( const char* configFileName )
{    
    logfile().pushInfo(__FILE__,__FUNCTION__);
    ifstream config;
    config.open(configFileName, fstream::in/* | fstream::nocreate*/);
    if ( !config.is_open() )
    {
        stringstream ss;
        ss << "Error opening config file '" << configFileName << "'" << ". Using default configuration\n";
        logfile().write(includelog::Init, ss.str());
        logfile().popInfo();
        return false; // use default value
    }

    stringstream ss;
    ss << "Parsing configuration in file: '" << configFileName << "'\n";
    logfile().write(includelog::Init, ss.str());
    
    /* static avoid stack overflow */
    static char option[8192]; /* 8 Kbytes max line */

    char* value;
    
    while ( !config.eof() )
    {
        config.getline(option, sizeof(option));        
        
        value = option;
        
        while ( *value != '\0' && *value != '=' )
            value++;
        
        if ( *value == '=' )
        {
            *value = '\0';
            value++; // skip '='
            
            // opt: has an option ; value: contains the value for this option
            if ( EQ(option,"outputFile") )
            {
                logfile().write(includelog::Init, "Setting output file to: ");
                if ( EQ(value,"NULL") || EQ(value,"null") )
                {
                    GLILogFlags[LOG_TRACE] = false;
                    logfile().write(includelog::Init, "NULL\n", false);
                }
                else
                {

                    strcpy(outputTraceFile, value);
                    GLILogFlags[LOG_TRACE] = true;
                    ss.str("");
                    ss << value << "\n";
                    logfile().write(includelog::Init, ss.str(), false);
                }
            }
            else if ( EQ(option,"traceReturn") )
            {
                if ( EQ(value, "0") )
                {
                    GLInterceptor::setReturnTraced(false);
                    logfile().write(includelog::Init, "Trace RETURN set to FALSE\n");
                }
                else
                {
                    GLInterceptor::setReturnTraced(true);
                    logfile().write(includelog::Init, "Trace RETURN set to FALSE\n");
                }
            }
            else if ( EQ(option,"format") )
            {
                if ( EQ(value,"text") || EQ(value,"TEXT") )
                {
                    tw.setMode(TraceWriter::text);
                    logfile().write(includelog::Init, "Trace format set to TEXT\n");
                }
                else if ( EQ(value,"binary") || EQ(value,"BINARY") )
                {
                    tw.setMode(TraceWriter::binary);
                    logfile().write(includelog::Init, "Trace format set to BINARY\n");
                }
                else if ( EQ(value,"hex") || EQ(value,"HEX") )
                {
                    tw.setMode(TraceWriter::hex);
                    logfile().write(includelog::Init, "Trace format set to HEX\n");
                }
                else
                {
                    logfile().write(includelog::Panic, "Trace format unknown [Exiting...]\n");
                    panic("GLInterceptor", "parseConfigFile", "trace format unknown");
                }
            }
            else if ( EQ(option,"frameStats") )
            {
                if ( EQ(value,"NULL") || EQ(value,"null") )
                {
                    logfile().write(includelog::Init, "Frame Statistics DISABLED (null)\n");
                    statManager.setPerFrameStats(false);
                }
                else
                {
                    ss.str("");
                    ss << "Frame Statistics file set to '" << value << "'\n";
                    logfile().write(includelog::Init, ss.str());
                    statManager.setPerFrameStats(true);
                    strcpy(statsPerFrameFile, value);
                }
            }
            else if ( EQ(option, "batchStats") )
            {
                if ( EQ(value,"NULL") || EQ(value,"null") )
                {
                    logfile().write(includelog::Init, "Batch Statistics DISABLED (null)\n");
                    statManager.setPerBatchStats(false);
                }
                else
                {
                    ss.str("");
                    ss << "Batch Statistics file set to '" << value << "'\n";
                    logfile().write(includelog::Init, ss.str());
                    statManager.setPerBatchStats(true);
                    strcpy(statsPerBatchFile, value);
                }
            }
            else if ( EQ(option,"statsTotalFile") )
            {
                logfile().write(includelog::Init, "Warning. Stats Total file option unsupported [IGNORED]\n");
                if ( EQ(value,"NULL") || EQ(value,"null") )
                    GLILogFlags[LOG_STATS_TOTAL] = false;
                else
                    strcpy(statsTotalFile, value);
            }
            else if ( EQ(option,"logBuffers") )
            {
                if ( EQ(value,"0") )
                {
                    logfile().write(includelog::Init, "Data buffers logging DISABLED\n");
                    GLILogFlags[LOG_BUFFERS] = false;
                }
                else
                {
                    logfile().write(includelog::Init, "Data buffers logging ENABLED\n");
                    GLILogFlags[LOG_BUFFERS] = true;
                }
            }
            else if ( EQ(option,"compressedOutput") )
            {
                if ( EQ(value,"1") )
                {
                    logfile().write(includelog::Init, "Compressed output ENABLED\n");
                    compressedOutput = true;
                }
                else
                {
                    logfile().write(includelog::Init, "Compressed output DISABLED\n");
                    compressedOutput = false;
                }
            }
            else if ( EQ(option,"firstFrame") )
            {
                firstFrame = atoi(value);
                firstFrame = ( firstFrame > 0 ? firstFrame : 0 ); // CLAMP to [1..INFINITE)
                ss.str("");
                ss << "First frame to be traced: " << firstFrame << "\n";
                logfile().write(includelog::Init, ss.str());
            }
            else if ( EQ(option,"lastFrame") )
            {
                lastFrame = atoi(value);
                lastFrame = ( lastFrame < 0 ? -1 : lastFrame ); // CLAMP to [0..INFINITE)
                ss.str("");
                ss << "Last frame to be traced: " << lastFrame << "\n";
                logfile().write(includelog::Init, ss.str());
            }
            else if ( EQ(option,"hackDLL") )
            {
                if ( strncmp(value,"1",1) == 0 )
                {
                    logfile().write(includelog::Init, "HackDLL option set to ENABLED\n");
                    GLInterceptor::setHackMode(true);
                }
                else 
                {
                    logfile().write(includelog::Init, "HackDLL option set to DISABLED\n");
                    GLInterceptor::setHackMode(false);
                }
            }
            else if ( EQ(option,"executeCalls") )
            {
                if ( EQ(value,"0") )
                {
                    logfile().write(includelog::Init, "Execute OpenGL calls ONLY set to FALSE [trace only]\n");
                    GLInterceptor::setExecuteRealCalls(false);
                }
                else
                {
                    logfile().write(includelog::Init, "Execute OpenGL calls ONLY set to TRUE [execute & trace]\n");
                    GLInterceptor::setExecuteRealCalls(true);
                }
            }
            else if ( EQ(option,"splitStats") )
            {
                splitLevel = atoi(value);
                
                if ( splitLevel <= 0 )
                    logfile().write(includelog::Init, "SplitStats option set to NO SPLIT\n");
                else
                {
                    ss.str("");
                    ss << "SplitStats option set to " << splitLevel << " frames per file\n";
                    logfile().write(includelog::Init, ss.str());
                }
            }
            else if ( EQ(option, "dumpMode") )
            {
                if ( EQ(value, "VERTICAL") || EQ(value,"vertical") )
                {
                    logfile().write(includelog::Init, "Statistics dump mode set to VERTICAL\n");
                    vertical = true;
                }
                else
                {
                    logfile().write(includelog::Init, "Statistics dump mode set to HORIZONTAL\n");
                    vertical = false;
                }
            }
            else if ( EQ(option, "bufferCache") )
            {
                cacheSz = atoi(value);
                if ( cacheSz <= 0 )
                    cacheSz = 100;
                else if ( cacheSz > 100000 )
                    cacheSz = 100000;

                ss.str("");
                ss << "Buffer cache size option set to " << cacheSz << " buffers\n";
                logfile().write(includelog::Init, ss.str());
            }
            else if ( EQ(option, "allowUndefinedBuffers") )
            {
                if ( EQ(value,"1") )
                {
                    logfile().write(includelog::Init, "Allow undefined buffers option set to TRUE\n");
                    acceptDeferredBuffers = true;
                }
                else
                {
                    logfile().write(includelog::Init, "Allow undefined buffers option set to FALSE\n");
                    acceptDeferredBuffers = false;
                }

            }
            else if ( EQ(option, "skipStatsFrom") )
            {
                char funcName[256];
                
                while( *value == ' ' || *value == '\t' ) 
                    value++; /* skip whites */

                /* parse function names */
                while ( *value != 0 )
                {
                    while( *value == ' ' || *value == '\t' ) 
                        value++; /* skip whites */

                    int ii = 0;

                    while ( *value != 0 && *value != ' ' && *value != '\t' )
                    {
                        funcName[ii++] = *value;
                        value++;
                    }
                    funcName[ii] = 0;

                    /* disable call */
                    std::vector<APICall>* v = GLResolver::getFunctionNameMatching(funcName);
                    if ( v != NULL )
                    {
                        ss.str("");
                        ss << "Disabling stats of functions that match with pattern '" << funcName << "' ...\n";
                        logfile().write(includelog::Init, ss.str());
                        int i;
                        for ( i = 0; i < v->size(); i++ )
                        {
                            GLInterceptor::statManager.setStatCall((*v)[i], false);
                            const char* str = GLResolver::getFunctionName((*v)[i]);
                            ss.str("");
                            ss << "Warning. Stat counting from '" << str << "' will be ignored\n";
                            logfile().write(includelog::Init, ss.str());
                        }
                        ss.str("");
                        ss << "End of functions that match with the pattern '" << funcName << "'\n";
                        logfile().write(includelog::Init, ss.str());
                        delete v;
                    }
                    else
                    {
                        APICall apicall = GLResolver::getFunctionID(funcName);
                        if ( apicall != APICall_UNDECLARED )
                        {
                            GLInterceptor::statManager.setStatCall(apicall, false);
                            ss.str("");
                            ss << "Warning. Stat counting from '" << funcName << "' will be ignored\n";
                            logfile().write(includelog::Init, ss.str());
                        }
                        else
                        {
                            ss.str("");
                            ss << "Warning. Trying to ignoring a non-existent call (or pattern): '" << funcName << "'\n";
                            logfile().write(includelog::Init, ss.str());
                        }
                    }
                    
                    while( *value == ' ' || *value == '\t' ) 
                        value++; /* skip whites */
                }
            }
            else if ( EQ(option, "getStatsFrom") )
            {    
                char funcName[256];
                
                while( *value == ' ' || *value == '\t' ) 
                    value++; /* skip whites */

                /* parse function names */
                while ( *value != 0 )
                {
                    while( *value == ' ' || *value == '\t' ) 
                        value++; /* skip whites */

                    int ii = 0;

                    while ( *value != 0 && *value != ' ' && *value != '\t' )
                    {
                        funcName[ii++] = *value;
                        value++;
                    }
                    funcName[ii] = 0;

                    /* enable call */
                    std::vector<APICall>* v = GLResolver::getFunctionNameMatching(funcName);
                    if ( v != NULL )
                    {
                        ss.str("");
                        ss << "Enabling stats of functions that match with pattern '" << funcName << "' ...\n";
                        logfile().write(includelog::Init, ss.str());
                        int i;
                        for ( i = 0; i < v->size(); i++ )
                        {
                            GLInterceptor::statManager.setStatCall((*v)[i], true);
                            const char* str = GLResolver::getFunctionName((*v)[i]);
                            ss.str("");
                            ss << "Stat counting from '" << str << "' enabled\n";
                            logfile().write(includelog::Init, ss.str());
                        }
                        ss.str("");
                        ss << "End of functions that match with the pattern '" << funcName << "'\n";
                        logfile().write(includelog::Init, ss.str());
                        delete v;
                    }
                    else
                    {
                        APICall apicall = GLResolver::getFunctionID(funcName);
                        if ( apicall != APICall_UNDECLARED )
                        {
                            GLInterceptor::statManager.setStatCall(apicall, true);
                            ss.str("");
                            ss << "Stat counting from '" << funcName << "' enabled\n";
                            logfile().write(includelog::Init, ss.str());
                        }
                        else
                        {
                            ss.str("");
                            ss << "Warning. Trying to enabling a non-existent call (or pattern): '" << funcName << "'\n";
                            logfile().write(includelog::Warning, ss.str());
                        }
                    }
                    
                    while( *value == ' ' || *value == '\t' ) 
                        value++; /* skip whites */
                }
            }
        }
    }
    
    logfile().popInfo();
    return true;
}


void GLInterceptor::setLogFlag( GLI_LOGFLAG flag, bool enabled )
{
    GLILogFlags[flag] = enabled;
}

bool GLInterceptor::isLogFlagEnabled( GLI_LOGFLAG flag )
{
    return GLILogFlags[flag];
}

const char* GLInterceptor::getOutputFile( GLI_OFILEFLAG flag )
{
    switch ( flag )
    {
        case FILE_TRACEFILE :
            return outputTraceFile;
        case FILE_STATS_PER_FRAME :
            return statsPerFrameFile;
        case FILE_STATS_PER_BATCH :
            return statsPerBatchFile;
        case FILE_STATS_TOTAL :
            return statsTotalFile;
    }
    return NULL;
}


int GLInterceptor::getFirstFrame()
{
    return firstFrame;
}

int GLInterceptor::getLastFrame()
{
    return lastFrame;
}

TraceWriter::TW_MODE GLInterceptor::getInitialTWMode()
{
    return initialTWMode;
}


bool GLInterceptor::isHackMode()
{
    return hackMode;
}

void GLInterceptor::setHackMode( bool mode )
{
    hackMode = mode;
}

bool GLInterceptor::executeRealGLCalls()
{
    return executeCalls;
}

void GLInterceptor::setExecuteRealCalls( bool executeCalls_ )
{
    executeCalls = executeCalls_;
}

void GLInterceptor::setAppResolution(int width, int height)
{
    resWidth = width;
    resHeight = height;
}

void GLInterceptor::getAppResolution(int& width, int& height)
{
    width = resWidth;
    height = resHeight;
}


string GLInterceptor::getInternalInfo()
{
    static string internalInfo(
        "GLInterceptor " GLI_VER "\n"
        "Built: "  __DATE__ " " __TIME__ "\n");
    return internalInfo;
}