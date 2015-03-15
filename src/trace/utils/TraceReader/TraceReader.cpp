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

#include "TraceReader.h"
#include <sstream>
//#include <fstream> /* upgraded */
//#include <cstdio>
#include "IncludeLog.h"
#include <cstring>
#include <cstdlib>

#ifndef USE_FSTREAM_H

#include <iostream>
    using namespace std;
#endif

#define IS_WHITE(c) ((c)==' ' || (c)=='\t')
#define IS_MARK(c) ((c)==',' || (c)==')')


using includelog::logfile; // Make log object visible

TraceReader::TraceReader() : mode(TraceReader::hex), line(1), directivesOn(true), batchesAsFrames(false), specialStr("")
{
}

TraceReader::TR_MODE TraceReader::getMode() const
{
    return mode;
}

void TraceReader::skipWhites()
{
    GLOBALPROFILER_ENTERREGION("skipWhites", "TraceReader", "skipWhites")
    
    if ( mode == binary )
        return ;

    if ( trEOS() )
        return ;

    char c = (char)trPeek();

    while ( !trEOS() && IS_WHITE(c) )
    {
        trIgnore();
        c = (char)trPeek();
    }

    GLOBALPROFILER_EXITREGION()
}

bool TraceReader::open( const char* traceFile )
{
    strcpy(filePath, traceFile);
    return trOpen(traceFile);
}



bool TraceReader::open( const char* traceFile,
                        const char* bufferDescriptorsFile,
                        const char* memoryFile )
{
    if ( TraceReader::open(traceFile ) )
    {
        bm.open(bufferDescriptorsFile, memoryFile, BufferManager::Loading);
        return true;
    }
    return false;
}


bool TraceReader::parseEnum( unsigned int& value )
{
    GLOBALPROFILER_ENTERREGION("parseEnum", "TraceReader", "parseEnum")
    value = 0; // reset
    char c = trPeek();
    if ( '0' <= c && c <= '9'  )
    {
        /* HEX or DEC value */
        bool success = parseNumber(value);
        
        GLOBALPROFILER_EXITREGION()
        
        return success;
    }
    else
    {
        /* GL Constant */
        char temp[256];
        int i = 0;
        c = trPeek();
        while (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
               ('0' <= c && c <= '9') || (c == '_'))
        {
            trIgnore();
            temp[i++] = c;
            c = trPeek();
        }
        temp[i] = '\0';

        if ( (i = GLResolver::getConstantValue(temp)) < 0 )
        {
            char buffer[256];
            sprintf(buffer,"Unknown constant string: \"%s\" in line: %d", temp, line );
            panic("TraceReader", "parseEnum()",buffer);
        }
        value = (unsigned int)i;
        GLOBALPROFILER_EXITREGION()
        return true;
    }
}

bool TraceReader::parseNumber( unsigned int& value )
{
    GLOBALPROFILER_ENTERREGION("parseNumber", "TraceReader", "parseNumber")
    // New and faster but not safer implementation...
    char c;
    value = 0;

    c = trPeek();
    if ( c == '0' )
    {
        trIgnore();
        c = trPeek();
        if ( c == 'X' || c == 'x' )
        {
            // Assume hex value 0x...
            trIgnore(); // ignore 0 and x
            f >> std::hex;
            trReadFormated(&value);
            f >> std::dec;
        }
        // else (assume 0)
        GLOBALPROFILER_EXITREGION()
        return true;
    }
    else if ( '1' <= c && c <= '9' )
    {
        trReadFormated(&value);
        GLOBALPROFILER_EXITREGION()
        return true;
    }
    else if ( c == '-' )
    {
        trIgnore(); // ignore '-'
        trReadFormated(&value);
        value = -value;
        GLOBALPROFILER_EXITREGION()
        return true;
    }
    else
    {
        stringstream ss;
        ss << "Number expected... Last APICall: " << GLResolver::getFunctionName(lastCall);
        panic("TraceReader", "parseNumber", ss.str().c_str());
    }


    panic("TraceReader", "parseNumber", "Legacy code reached");

    // Old version - more robust

    // debug
    char myBuf[256];
    // fdebug


    value = 0;

    c = trPeek();

    if ( '0' <= c && c <= '9' )
    {
        trIgnore(); // remove caracter from input stream

        value = (int)c - 48;

        c = (char)trPeek();

        if ( c == 'x' || c == 'X' ) // it is an hex value
        {

            unsigned int partial = 0;

            trIgnore(); // remove caracter from input stream ( it is a 'x' or 'X'

            c = (char)trPeek();

            while (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'))
            {
                trIgnore(); // remove caracter from input stream

                if ( 'a' <= c && c <= 'f' )
                    partial = c - 'a' + 10;
                else if ( 'A' <= c && c <= 'F' )
                    partial = c - 'A' + 10;
                else
                    partial = (int)c - 48;

                value = (value << 4) + partial; /* value * 16 + partial */

                c = (char)trPeek(); // next
            }
        }
        else if ( '0' <= c && c <= '9' )
        {
            value = value * 10 + ((int)c - 48);

            trIgnore();

            c = (char)trPeek();
            while ( '0' <= c && c <= '9' )
            {
                trIgnore();

                value = value * 10 + ((int)c - 48);

                c = (char)trPeek();
            }
        }

        return true; // some value has been parsed
    }

    sprintf(myBuf,"Error parsing a number in line %d. First character found before parsing: '%c'",line,c);
    panic("TraceReader", "parseNumber()",myBuf);
    return false; // not a number
}


APICall TraceReader::parseApiCall()
{
    GLOBALPROFILER_ENTERREGION("parseApiCall", "TraceReader", "parseApiCall")
    using namespace std;

    if ( trEOS() )
    {
        GLOBALPROFILER_EXITREGION()
        return APICall_UNDECLARED;
    }
    
    if ( mode == binary )
    {
        unsigned short apiID;
        trRead(&apiID,2);
        GLOBALPROFILER_EXITREGION()
        return (APICall)apiID;
    }

    char c;

    while ( !trEOS() )
    {
        skipWhites();

        c = trPeek();

        if ( c != '#' && c != '\n' )
            break;

        if ( c == '\n' )
        {
            line++;
            trIgnore();
        }
        else // must be '#'
        {
            // check if it is a command
            trIgnore(); // skip #
            c = trPeek();
            if ( c != '!' )
            {
                // It's not a comand (#!), just a comment (#)
                skipLine();
                continue;
            }

            // It is command (Process command)
            trIgnore(); // Ignore '!'
            skipWhites();
            char command[1024];
            int i = 0;
            c = trPeek();
            while ( c != ' ' && c != '\n' && c != '\t' )
            {
                command[i] = c;
                trIgnore();
                c = trPeek();
                i++;
            }
            command[i] = 0;
            if ( strcmp(command, "PRINT") == 0 )
            {
                skipWhites();
                trGetline(command, sizeof(command));
                cout << "output:> " << command << endl;
                popup("Output PRINT command", command);
            }
            else if ( strcmp(command, "END") == 0 && directivesOn )
            {
                cout << "output:> Finishing trace (END command found)" << endl;
                skipLine();
                GLOBALPROFILER_EXITREGION()
                return APICall_UNDECLARED; // Force finishing trace
            }
            else if ( strcmp(command, "SWAPBUFFERS") == 0 && directivesOn )
            {
                cout << "output:> sending SWAPBUFFERS command" << endl;
                skipLine();
                GLOBALPROFILER_EXITREGION()
                return APICall_wglSwapBuffers;
            }
            else if ( strcmp(command, "BATCHES_AS_FRAMES") == 0 )
            {
                skipWhites();
                trGetline(command, sizeof(command));
                if ( strncmp(command, "ON",2) == 0 )
                {
                    cout << "output:> BATCHES_AS_FRAMES enabled (ON)" << endl;
                    batchesAsFrames = true;
                }
                else
                {
                    cout << "output:> BATCHES_AS_FRAMES disabled (OFF)" << endl;
                    batchesAsFrames = false;
                }
            }
            else if ( strcmp(command, "LABEL") == 0 && directivesOn )
            {
                cout << "output:> LABEL command not supported yet" << endl;
                skipLine();
            }
            else if ( strcmp(command, "LOOP") == 0 && directivesOn )
            {
                cout << "output:> LOOP command not supported yet" << endl;
                skipLine();
            }
            else if ( strcmp(command, "DUMP_CTX") == 0 )
            {
                cout << "output:> sending DUMP_CTX command..." << endl;
                specialStr  = "DUMP_CTX";
                skipLine();
                GLOBALPROFILER_EXITREGION()
                return APICall_SPECIAL; // send a SPECIAL message
            }
            else if ( strcmp(command, "DUMP_STENCIL") == 0 )
            {
                cout << "output:> sending DUMP_STENCIL command..." << endl;
                specialStr  = "DUMP_STENCIL";
                skipLine();
                GLOBALPROFILER_EXITREGION()
                return APICall_SPECIAL; // send a SPECIAL message
            }
            else
                skipLine(); // ignore unknown command
        }
    }


    if ( trEOS() )
    {
        GLOBALPROFILER_EXITREGION()
        return APICall_UNDECLARED;
    }

    char buffer[256];


    // Improved version
    trGetline(buffer, sizeof(buffer),'(');

    GPU_TRACE_LOG(
        stringstream ss;
        ss << "Parsing call " << buffer << " at line " << line << "\n";
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Debug, ss.str());
        logfile().popInfo();
    )

    GLOBALPROFILER_ENTERREGION("APICallMap", "TraceReader", "parseAPICall")
    map<string,APICall>::iterator it = translatedCalls.find(buffer);
    GLOBALPROFILER_EXITREGION()
    if ( it != translatedCalls.end() )
    {
        //popup("Processing call", buffer);
        GLOBALPROFILER_EXITREGION()
        return (lastCall = it->second);
    }

    GLOBALPROFILER_ENTERREGION("resolveAPICall", "TraceReader", "parseAPICall")
    lastCall = GLResolver::getFunctionID(buffer);
    translatedCalls.insert(make_pair(string(buffer), lastCall));;
    GLOBALPROFILER_EXITREGION()
    if ( lastCall == APICall_UNDECLARED )
    {
        char temp[512];
        sprintf(temp, "APICall unknown: '%s' in line %d", buffer, line);
        panic("TraceReader", "parseAPICall()", temp);
    }
    //popup("Processing call", buffer);
    GLOBALPROFILER_EXITREGION()
    return lastCall;

    // Old version...

    panic("TraceReader", "parseAPICall", "Legacy code reached.");
    
    int i = 0;

    while ( !trEOS() && i < (sizeof(buffer)-1) && (c = trPeek()) != '(' )
    {
        if (('0'<=c && c<='9') || ('a'<=c && c<='z') || ('A'<=c && c <='Z'))
            buffer[i++] = c;
        else
        {
            return APICall_UNDECLARED; /* Hacking for PFC */
            sprintf( buffer, "char %c cannot be part of an API call (line %d)", c, line);
            panic("TraceReader", "parseAPICall()", buffer);
        }
        trIgnore();
    }

    if ( c != '(' )
    {
        buffer[i] = 0;
        char temp[2*sizeof(buffer)];
        sprintf(temp, "APICall name too long: '%s' in line", buffer, line);
        panic("TraceReader","parseAPICall()", temp);
    }

    trIgnore(); /* skip '(' */
    buffer[i] = 0;

    lastCall = GLResolver::getFunctionID(buffer);

    if ( lastCall == APICall_UNDECLARED )
    {
        char temp[512];
        sprintf(temp,  "APICall unknown: '%s' in line %d", buffer, line);
        panic("TraceReader", "parseAPICall()", temp);
    }
    return lastCall;
}


bool TraceReader::readOring( unsigned int* value)
{
    GLOBALPROFILER_ENTERREGION("readOring", "TraceReader", "readerOring")
    if ( mode == binary )//|| mode == hex )
    {
        bool success = readEnum(value);
        GLOBALPROFILER_EXITREGION()
        return success;
    }
    
    skipWhites();
    char c;

    /* text mode */
    *value = 0;
    unsigned int temp = 0;
    while  ( true )
    {
        parseEnum(temp);
        skipWhites();
        c = (char)trPeek();
        if ( c == ')' || c == ',' )
        {
            /* end of list of bitfields */
            *value |= temp;
            trIgnore(); /* skip mark */
            GLOBALPROFILER_EXITREGION()
            return true;
        }
        else if ( c == '|' )
        {
            *value |= temp;
            trIgnore();
            skipWhites();
        }
        else
        {
            char aux[256];
            sprintf(aux, "Unexpected mark '%c' found in line %d", c, line);
            panic("TraceReader", "readOring()", aux);
        }
    }
    GLOBALPROFILER_EXITREGION()
}

bool TraceReader::readEnum( unsigned int* value)
{
    GLOBALPROFILER_ENTERREGION("readEnum", "TraceReader", "readEnum")

    if ( mode == binary )
    {
        trRead(value,4);
        GLOBALPROFILER_EXITREGION()
        return true;
    }

    skipWhites();

    if ( mode == text || mode == hex )
    {
        parseEnum(*value); /* supports both HEX & DEC modes */
        skipWhites();
        char c;
        trGet(c);
        if ( c == ',' || c == '}' || c == ')')
        {
            GLOBALPROFILER_EXITREGION()
            return true;
        }
        char myBuf[256];
        sprintf(myBuf,"Error reading value in line %d. Expected mark not found", line);
        panic("TraceReader", "readEnum()",myBuf);
    }

    GLOBALPROFILER_EXITREGION()
    return false;

    // hex
    //return TraceReader::read(value);
}

/*
bool TraceReader::readPFD(void **ptr, char* data, int dataSize, ArrayTextType att )
{
    return readArray(ptr, data, dataSize, att);
}
*/

// 'ptr' will point data returned ( to buffer index or to data array )
// 'data' used for store text array or inlined array contents
// if it is a buffer index data do ot contain anything
// Remember: u must use ptr for extract data
bool TraceReader::readArray( void** ptr, char* data, int dataSize, ArrayTextType att )
{
    GLOBALPROFILER_ENTERREGION("readArray", "TraceReader", "readArray")
    BufferDescriptor* buf = NULL;
    unsigned int bufId = 1;

    /***************
     * Binary mode *
     ***************/

    if ( mode == binary )
        panic("TraceReader", "readArray", "Binary mode not supported yet");

    /******************
     * READABLE MODES *
     ******************/

    int i;
    char c;

    skipWhites();

    c = (char)trPeek();

    switch ( c )
    {
        case '{': // inlined array ( vector )
            trIgnore();

            if ( mode == hex )
            {
                for ( i = 0; ; i++ )
                {
                    if ( !parseNumber(*(((unsigned int*)data)+i)) )
                    {
                        panic("TraceReader", "readArray()", "Error parsing elem in inlined array");
                    }
                    else
                    {
                        c = (char)trPeek();
                        if ( c == '.' ) // it is a double number
                        {
                            trIgnore();
                            i++;
                            if ( !parseNumber(*(((unsigned int*)data)+i)) )
                            {
                                panic("TraceReader", "readArray()", "Error parsing elem (2nd part) in inlined array");
                            }
                        }
                    }

                    skipWhites();
                    trGet(c);
                    if ( c == ',' )
                        skipWhites(); /* go to next array item */
                    else if ( c == '}' ) /* End of array */
                    {
                        trGet(c);
                        if ( c == ',' ) /* array is not the last parameter */
                        {
                            skipWhites();
                            break; /* end of parsing. go out from loop */
                        }
                        else if ( c == ')' ) /* array is the last parameter */
                            break; /* end of parsing. go out from loop */
                        else
                        {
                            char temp[256];
                            sprintf(temp, "Error parsing array. Expected mark not found. Mark found: %c", c);
                            panic("TraceReader", "readArray()", temp);
                        }
                    }
                } /* end for */
            }
            else if ( mode == text )
            {
                for ( i = 0; ; i++ )
                {
                    skipWhites();
                    char cc = (char)trPeek();
                    if ( cc == ',' )
                        panic("TraceReader", "readArray", "Unexpected ',', a value was expected");
                    else if ( cc == '}' )
                        break; // end of parsing (empty array...)
                    else if ( cc != '-' && cc != '+' && ('0' > cc || cc > '9') )
                    {
                        char msg[256];
                        sprintf(msg, "Unexpected character: '%c', a value was expected", cc);
                        panic("TraceReader", "readArray", msg);
                    }

                    switch ( att )
                    {
                        case TEXT_BYTE:
                            trReadFormated(data+i);
                            break;
                        case TEXT_SHORT:
                            trReadFormated((((unsigned short *)data)+i));
                            break;
                        case TEXT_INT:
                            trReadFormated((((unsigned int *)data)+i));
                            break;
                        case TEXT_FLOAT:
                            trReadFormated((((float *)data)+i));
                            break;
                        case TEXT_DOUBLE:
                            trReadFormated((((double *)data)+i));
                            break;
                        default:
                            panic("TraceReader", "readArray()","TEXT_TYPE unknown");
                    }

                    skipWhites();

                    trGet(c);

                    if ( c == ',' )
                        skipWhites(); /* go to next array item */
                    else if ( c == '}' ) /* End of array */
                    {
                        trGet(c);
                        if ( c == ',' ) /* array is not the last parameter */
                        {
                            skipWhites();
                            break; /* end of parsing. go out from loop */
                        }
                        else if ( c == ')' ) /* array is the last parameter */
                            break; /* end of parsing. go out from loop */
                        else
                        {
                            panic("TraceReader", "readArray()", "Error parsing array. Expected mark not found");
                        }
                    }
                    else
                    {
                        char msg[256];
                        sprintf(msg, "Unexpected character: '%c'. simbols ',' or '}' expected", c);
                        panic("TraceReader", "readArray", msg);
                    }
                }
            }
            *ptr = (void *)data; /* return pointer */
            GLOBALPROFILER_EXITREGION()
            return true;
        case '*': // buffer index
            trIgnore();
            parseNumber(bufId);

            buf = bm.find(bufId);
            if ( buf == 0 )
            {
                stringstream ss;
                ss << "Buffer descriptor: " << bufId << " not found";
                panic("TraceReader", "readArray", ss.str().c_str());
            }

            skipWhites();
            trGet(c);
            if ( c != ',' && c != ')' )
            {
                char temp[256];
                sprintf(temp, "Expected mark not found after reading buffer identifier. Read: '%c'", c);
                panic("TraceReader","readArray()", temp);
            }

            *ptr = (void*)buf->getData();

            /*
            if ( *ptr == 0 )
            {
                stringstream ss;
                ss << "Buffer descriptor: " << buf->getID() << " returning NULL with getData()";
                popup("TraceReader::readArray", ss.str().c_str());
                //panic("TraceReader", "readArray", "Memory data not found!");
            }
            */

            GLOBALPROFILER_EXITREGION()
            return true;
        case '"': // text array
            trIgnore();
            trGetline(data,dataSize,'"');
            {
                //stringstream ss;
                //ss << "Data to parse: '" << data << "' linesBefore = " << line;
                {
                    for ( int i = 0; data[i]; i++ )
                    {
                        if ( data[i] == '\n' ) line++;
                    }
                }

                //ss << " linesAfter = " << line << "\n";
                //logfile().write(includelog::Debug, ss.str());
            }
            skipWhites();
            trGet(c);
            if ( c != ',' && c != ')' )
            {
                char temp[256];
                sprintf(temp, "Expected mark not found after reading a text array. Read: '%c'", c);
                panic("TraceReader","readArray()", temp);
            }
            *ptr = (void *)data;
            GLOBALPROFILER_EXITREGION()
            return true;
        default:
            if ( '0' <= c && c <= '9' )
            {
                /* using a pointer as an offset or maybe a null pointer */
                parseNumber(*((unsigned int *)ptr));
                skipWhites();
                trGet(c);
                if ( c != ',' && c != ')' )
                    panic("TraceReader","readArray()", "Expected mark not found after reading an offset (or null pointer)");
                GLOBALPROFILER_EXITREGION()
                return true;

            }
            else
            {
                stringstream ss;
                ss << "Unknown start buffer. Token: \"" << c << "\" in line: " << line
                    << " Last GL Call: " << GLResolver::getFunctionName(lastCall);
                panic("TraceReader", "readArray()", ss.str().c_str()); /* should not happen ever */
            }
    }
    GLOBALPROFILER_EXITREGION()
    return false;
}

bool TraceReader::skipLine()
{
    GLOBALPROFILER_ENTERREGION("skipLine", "TraceReader", "skipLine")
    
    if ( mode == binary )
    {
        GLOBALPROFILER_EXITREGION()
        return true;
    }
    
    char c;
    if ( trEOS() )
    {
        GLOBALPROFILER_EXITREGION()
        return true;
    }
    
    trGet(c);
    while ( !trEOS() && c != '\n' )
        trGet(c);

    line++;

    GLOBALPROFILER_EXITREGION()
    return true;
}

bool TraceReader::readBoolean( unsigned char* value)
{
    GLOBALPROFILER_ENTERREGION("readBoolean", "TraceReader", "readBoolean")
    
    if ( mode == binary )
        panic("TraceReader", "readBoolean", "Binary reads not implemented yet");

    unsigned char b;

    skipWhites();
    trReadFormated(&b);
    skipWhites();

    char c;
    trGet(c); // skip mark char

    if ( c == ',' || c == '}' || c == ')' )
        ;
    else
    {
        char buffer[256];
        sprintf(buffer,"Error reading value, current char is: '%c'",c);
        panic("TraceReader", "readBoolean()",buffer);
        GLOBALPROFILER_EXITREGION()
        return false;
    }


    if ( b == '0' )
        *value = 0;
    else if (b == '1')
        *value = 1;
    else
        *value = b;
    GLOBALPROFILER_EXITREGION()
    return true;
}


// double representation in a tracefile 0xAAAAAAAA.0xBBBBBBBBB
bool TraceReader::readDouble( double *value )
{
    GLOBALPROFILER_ENTERREGION("readDouble", "TraceReader", "readDouble")
    char buffer[256];

    unsigned int a;
    unsigned int b;
    unsigned int *ptrU;
    char c;

    if ( mode == binary )
    {
        // 8 raw bytes
        trRead(value,sizeof(double));
        GLOBALPROFILER_EXITREGION()
        return true;
    }

    if ( mode == text )
    {
        trReadFormated(value);
        skipWhites();
        trGet(c);
        if ( c == ',' || c == '}' || c == ')' )
        {
            GLOBALPROFILER_EXITREGION()
            return true;
        }            
        sprintf(buffer,"Error reading value in line %d, current char is: '%c'",line,c);
        panic("TraceReader", "readDouble()",buffer);
        return false;
    }


    *value = 0; // reset ( for debug purpouse )

    skipWhites(); // skip previous whites

    if ( mode == hex )
    {
        if ( !parseNumber(a) ) {
            panic("TraceReader", "readDouble()","Error parsing 1st HEX");
            return false;
        }
        trGet(c);
        if ( c != '.' ) {
            panic("TraceReader", "readDouble()","Error parsing '.' character");
            return false;
        }
        if ( !parseNumber(b) )
        {
            panic("TraceReader", "readDouble()","Error parsing 2nd HEX");
            return false;
        }

        ptrU = (unsigned int *)value;
        *ptrU = a;
        *(ptrU+1) = b;

        skipWhites();

        trGet(c); // skip mark char

        if ( c == ',' || c == '}' || c == ')' )
        {
            GLOBALPROFILER_EXITREGION()
            return true;
        }
        sprintf(buffer,"Error reading value, current char is: '%c'",c);
        panic("TraceReader", "readDouble()",buffer);
        return false;

    }
    panic("TraceReader" ,"readDouble()","Error reading value");
    return false; // text no supported
}


unsigned int TraceReader::currentLine() const
{
    return line;
}

#ifdef WIN32
    #define MSC8_LOCALE_FIX
#endif

#ifdef MSC8_LOCALE_FIX

class MSC8LocaleFix : public std::numpunct_byname<char>
{
    public:
        MSC8LocaleFix (const char *name)
            : std::numpunct_byname<char>(name)
        {
        }
    protected:
        virtual char_type do_thousand_sep() const
        {
            return '\0';
        }

        virtual string do_grouping() const
        {
            return "\0";
        }
};

#endif

bool TraceReader::trOpen( const char* file )
{
    logfile().pushInfo(__FILE__,__FUNCTION__);
    f.open(file, ios::binary | ios::in); // check if the file exists...
    if ( !f )
    {
        logfile().write(includelog::Init, "Input trace file not found");
        panic("TraceReader","trOpen()","Input trace file not found");
    }

    mode = readTraceFormat();

    f.close();

    if ( mode == binary )
        f.open(file, std::ios::binary | ios::in );
    else
        f.open(file, ios::binary | ios::in); /* text */

    if ( !f )
    {
        logfile().write(includelog::Init, "Input trace file not found (2)");
        panic("TraceReader","trOpen()","Input trace file not found (2)");
    }

    f.ignore(7); /* skip app string */

#ifdef MSC8_LOCALE_FIX
    std::locale loc2 (std::locale("C"), new MSC8LocaleFix("C"));
    f.imbue(loc2);
    locale loc = f.getloc();
#endif

    logfile().popInfo();
    return true;
}

int TraceReader::trPeek()
{
    GLOBALPROFILER_ENTERREGION("trPeek", "TraceReader", "trPeek")
    int p = f.peek();
    GLOBALPROFILER_EXITREGION()
    return p;
}

void TraceReader::trIgnore( int count )
{
    GLOBALPROFILER_ENTERREGION("trIgnore", "TraceReader", "trIgnore")
    f.ignore(count);
    GLOBALPROFILER_EXITREGION()
}


void TraceReader::trGet( char& c )
{
    GLOBALPROFILER_ENTERREGION("trGet", "TraceReader", "trGet")
    f.get(c);
    GLOBALPROFILER_EXITREGION()
}

void TraceReader::trGetline( char* buffer, int bufSize, char eol )
{
    GLOBALPROFILER_ENTERREGION("trGetLine", "TraceReader", "trGetLine")
    f.getline(buffer,bufSize,eol);
    GLOBALPROFILER_EXITREGION()
}

int TraceReader::trEOS()
{
    GLOBALPROFILER_ENTERREGION("trEOS", "TraceReader", "trEOS")
    bool endOfFile = f.eof();
    GLOBALPROFILER_EXITREGION()
    return endOfFile;
}

void TraceReader::trRead(void* data, int size)
{
    GLOBALPROFILER_ENTERREGION("trRead", "TraceReader", "trRead")
    f.read((char *)data,size);
    GLOBALPROFILER_EXITREGION()
}


long TraceReader::trGetPos()
{
    GLOBALPROFILER_ENTERREGION("trGetPos", "TraceReader", "trGetPos")
    long pos = f.tellg();
    GLOBALPROFILER_EXITREGION()
    return pos;
}


void TraceReader::trSetPos(long pos)
{
    GLOBALPROFILER_ENTERREGION("trSetPos", "TraceReader", "trSetPos")
    f.seekg(pos, ios::beg); // Absolute positioning
    GLOBALPROFILER_EXITREGION()
}



void TraceReader::readResolution(unsigned int& width, unsigned int& height)
{
    char line[256];
    unsigned int prevPos = f.tellg();
    //f.seekg(posRes);
    f.getline(line, sizeof(line));

    if ( strncmp("#! RES", line, 6) != 0 )
    {
        // There is not defined a resolution in the tracefile
        width = 0;
        height = 0;
        return ;
    }

    width = atoi(&line[6]);

    int i = 2;
    while ( line[i] != 0 && line[i] != 'x')
        i++;
    if ( line[i] = 0 )
        panic("TraceReader", "readResolution", "Height not found");

    height = atoi(&line[i+1]);

    f.seekg(prevPos, ios::beg);

    /*
    char msg[256];
    sprintf(msg, "hres = %d  vres = %d", width, height);
    MessageBox(NULL, msg, "Message", MB_OK);
    */
}

TraceReader::TR_MODE TraceReader::readTraceFormat()
{    
    char line[256];
    f.getline(line,sizeof(line));

    posRes = f.tellg(); // store resolution directive position

    //if (( strlen(line) == 6 )
    //{
    //    panic("TraceReader", "A readTraceFormat() - Version string unsupported", line);
    //}
    if ( strncmp("GLI0.",line,5) != 0 )
    {
        panic("TraceReader", " readTraceFormat() - Version string unsupported", line);
    }

    this->line++; /* go to line 2 */

    if ( line[5] == 'b' )
    {
        //popup("TraceReader::readTraceFormat()","Trace file in BINARY mode");
        return binary;
    }
    else if ( line[5] == 'h' )
    {
        //popup("TraceReader::readTraceFormat()","Trace file in HEX mode");
        return hex;
    }
    else if ( line[5] == 't' )
    {
        //popup("TraceReader::readTraceFormat()","Trace file in TEXT mode");
        return text;
    }

    panic("TraceReader", "readTraceFormat() - Format unsuppored", line);
    return text; /* dummy */
}


bool TraceReader::skipUnknown()
{
    GLOBALPROFILER_ENTERREGION("skipUnknown", "TraceReader", "skipUnknown")
    
    if ( binary ) // in binary unknown type occupies 0 bytes
    {
        GLOBALPROFILER_EXITREGION()
        return true;
    }
    else
    {
        // unknown format "U0xAAAAAA"
        // HEX or TEXT
        skipWhites();
        char c = trPeek();
        if ( c == 'U' || c == 'u' )
            trIgnore();
        else
        {
            //panic("TraceReader", "skipUnknown()","Incorrect format for unknown value");

        }

        while ( !trEOS() && c != ',' && c != ')' && c != '}' )
            trGet(c); // skip unknown
        if ( trEOS() )
        {
            panic("TraceReader", "skipUnknown()","Incorrect format for unknown value");
        }

        GLOBALPROFILER_EXITREGION()
        return true;
    }
}

bool TraceReader::skipResult()
{
    GLOBALPROFILER_ENTERREGION("skipResult", "TraceReader", "skipResult")
    if ( mode == binary )
    {
        GLOBALPROFILER_EXITREGION()
        return true;
    }
    
    // text or hex
    while ( trPeek() != '\n' ) // find return (eol)
        trIgnore();

    GLOBALPROFILER_EXITREGION()
    return true;
}

bool TraceReader::skipCall()
{
    GLOBALPROFILER_ENTERREGION("skipCall", "TraceReader", "skipCall")
    if ( mode == text || mode == hex )
    {
        char c;
        if ( trEOS() )
        {
            panic("TraceReader", "skipCall()","End of stream was reached");
        }
        trGet(c);
        while ( !trEOS() && c != ')' )
        {
            if ( c == '\n' )
                line++;
            trGet(c);
        }
        /* found ')' and skipped */
        skipWhites();

        if ( trEOS() )
        {
            GLOBALPROFILER_EXITREGION()
            return true;
        }
        
        if ( (char)trPeek() == '\n' )
        {
            line++;
            trIgnore(); /* call skipping finished */
            GLOBALPROFILER_EXITREGION()
            return true;
        }
        else if ( (char)trPeek() == '=' )
        {
            /* return value found. Must be skipped */
            trGet(c);
            while ( !trEOS() && c != '\n' )
                trGet(c);
            line++;
            GLOBALPROFILER_EXITREGION()
            return true;
        }
        else
        {
            if ( trPeek() == 13 ) // Skip 13 (NL)
            {
                trIgnore();
                GLOBALPROFILER_EXITREGION()
                return true;
            }
            char msg[256];
            sprintf(msg ,"Unexpected char: '%c' in tracefile line %d", (char)trPeek(), line);
            panic("TraceReader", "skipCall()",msg);
        }
        GLOBALPROFILER_EXITREGION()
        return true;
    }
    panic("TraceReader", "skipCall()","Binary mode still not supported");
    return true; /* dummy. Avoid compiler warnings */
}


long TraceReader::getCurrentTracePos()
{
    return trGetPos();
}

void TraceReader::setCurrentTracePos(long pos)
{
    trSetPos(pos);
}


