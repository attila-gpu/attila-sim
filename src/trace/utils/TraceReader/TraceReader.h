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

#ifndef TRACEREADER_H
    #define TRACEREADER_H

#include "support.h"
#include "GLResolver.h"
#include "BufferDescriptor.h"
#include <map>

//#define ENABLE_GLOBAL_PROFILER

#include "GlobalProfiler.h"

//#define USE_FSTREAM_H /* NOT USED ANYMORE: increase speed */
//#ifdef USE_FSTREAM_H
//    #include <fstream.h>
//#else
//    #include <fstream>
//#endif
#include "zfstream.h"

#include <cstdio>

#ifdef ENABLE_TRACE_LOG
    #define GPU_TRACE_LOG(a) {a}
#else
    #define GPU_TRACE_LOG(a) {}
#endif

/**
 *
 * How to parse a call
 *
 * glTranslated(0x80000000,0x80000000,0xc0a00000)
 *
 *
 * apicall = parseAPICall().id;
 *
 *  ...
 *
 *    case APICall_glTranslated:
 *
 *       STUB_glTranslated(tr)
 *       {
 *           tr.read(param[0].f32bit);
 *           tr.read(param[1].f32bit);
 *           tr.read(param[2].f32bit);
 *           tr.skipLine();
 *           jt.glTranslated(param[0].f32bit, param[1].f32bit, param[2].f32bit);
 *       }
 *       break;
 *
 *  ...
 *
 */
class TraceReader
{
public:

    /**
     * Gets and set the absolute position of a trace
     *
     * @warning It is required that the new position points to the start of a call (or comment).
     *          Otherwise the parse of the trace file will fail
     */
    long getCurrentTracePos();
    void setCurrentTracePos(long pos);

    void setDirectivesEnabled(bool enabled) { directivesOn = enabled; }
    bool areDirectivesEnabled() const { return directivesOn; }

    unsigned int currentLine() const;

    enum TR_MODE
    {
        binary,
        hex,
        text
    };

    enum ArrayTextType
    {
        TEXT_IGNORE, /* Default */
        TEXT_BYTE,
        TEXT_SHORT,
        TEXT_INT,
        TEXT_FLOAT,
        TEXT_DOUBLE
    };

    TraceReader();

    void setMode( TR_MODE mode );

    TR_MODE getMode() const;

    bool open( const char* traceFile );

    bool open( const char* traceFile,
               const char* bufferDescriptorsFile,
               const char* memoryFile );

    APICall parseApiCall();

    bool readEnum( unsigned int* value);

    bool readOring( unsigned int* value);


    /**
     * Format mode selector is only used in text mode
     *
     * FormatModeSelector options ( size per component )
     * 1 -> 1 byte
     * 2 -> 2 bytes
     * 4 -> 4 bytes
     * 8 -> 8 bytes
     */
    bool readArray( void** ptr, char* data, int dataSize, ArrayTextType att = TEXT_IGNORE );

    bool skipLine();

    /**
     * Skip current call and go to next one
     *
     * In binary mode that operation only works if we have read the APICALL id
     * and nothing else
     *
     * @see How binary traces are stored
     */
    bool skipCall();

    bool skipUnknown();

    bool skipResult();

    /**
     * Check if batches should be interpreted a frames
     */
    bool checkBatchesAsFrames() { return batchesAsFrames; }

    TR_MODE readTraceFormat();

    void readResolution(unsigned int& width, unsigned int& height);


    /**
     * Read any kind of value
     *
     * @warning Do not use with parameters wider than 32bits.
     *          Example do not use with double type. Use instead readDouble
     *
     * @note (for programmers). Every time you change the template implementation
     *       it is required a rebuild command (V.S 6.0). If rebuid is not performed
     *       specialized versions are not updated with the new template code :-)
     */
    template<typename T>
    bool read(T value)
    {
        GLOBALPROFILER_ENTERREGION("read", "", "")
        if ( mode == binary )
        {
            trRead(&value,sizeof(value));
            GLOBALPROFILER_EXITREGION()
            return true;
        }

        char myBuf[256];
        char c;
        unsigned int number = 0;

        skipWhites(); // skip previous whites

        if ( mode == hex )
        {
            if ( parseNumber(number) ) // HEX or DECIMAL
            {
                *value = *((T)(&number));
                skipWhites();
                trGet(c);
                if ( c == ',' || c == '}' || c == ')')
                {
                    GLOBALPROFILER_EXITREGION()
                    return true;
                }
                sprintf(myBuf,"Error reading value in line %d. Expected mark not found. Last GL Call: %s",
                    line, GLResolver::getFunctionName(lastCall));
                panic("TraceReader", "read(T value)",myBuf);
            }
            sprintf(myBuf,"Error parsing number in line %d",line);
            panic("TraceReader", "read(T value)", myBuf);
        }
        else if ( mode == text )
        {
            trReadFormated(value);
            skipWhites();
            trGet(c);
            if ( c == ',' || c == '}' || c == ')')
            {
                GLOBALPROFILER_EXITREGION()            
                return true;
            }
            sprintf(myBuf,"Error reading value in line %d. Expected mark not found", line);
            panic("TraceReader","read(T value)",myBuf);
        }

        sprintf(myBuf,"Error reading value in line %d. Mode not supported",line);
        panic("TraceReader", "read(T value)",myBuf);
        return false; // text mode not supported yet
    }

    // it could be implemented using partial specialization
    //( but for compatibility is implemented using a new function name )
    bool readDouble( double* value );

    // idem
    bool readBoolean( unsigned char* value );

    BufferManager& getBufferManager() { return bm; }

    /**
     * Allows commands to be observed from outside
     */
    std::string getSpecialString() const { return specialStr; }
    void resetSpecialString() { specialStr = ""; }


private:

    std::map<std::string, APICall> translatedCalls;

    APICall lastCall;

    std::string specialStr;

    BufferManager bm;

    bool batchesAsFrames;
    bool batchesAsFramesDirectiveFound;

    bool directivesOn;

    unsigned int line;

    char filePath[256];

    TR_MODE mode;

    unsigned int posRes;



//#ifdef USE_FSTREAM_H
//    ifstream f;
//#else
//    std::ifstream f;
//#endif

    gzifstream f;

    // parses HEX an DECIMAL values
    bool parseNumber( unsigned int& value );

    bool parseEnum( unsigned int& value );

    void skipWhites();

protected:

    /*
     * abstract access functions
     * All use 'tr' prefix
     *
     * Current implementation wraps a "simple" ifstream object
     */
    virtual bool trOpen( const char* file );

    virtual int trPeek();

    virtual void trIgnore( int count = 1 );

    virtual void trGet( char& c );

    virtual void trGetline( char* buffer, int bufSize, char eol = '\n' );

    virtual int trEOS();

    virtual long trGetPos();

    virtual void trSetPos(long pos);

    template<typename T>
    bool trReadFormated(T value)
    {
        GLOBALPROFILER_ENTERREGION("trReadFormated", "TraceReader", "trReadFormated")
        f >> (*value);
        GLOBALPROFILER_EXITREGION()
        return true;
    }

    // for binary reads
    virtual void trRead(void* data, int size);

};


#endif // TRACEREADER_H
