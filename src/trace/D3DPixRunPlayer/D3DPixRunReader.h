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

/**************************************************
PIXRun file format reader.

Author: Jos� Manuel Sol�s
**************************************************/

#ifndef __D3DPIXRUNREADER
#define __D3DPIXRUNREADER

#include <iostream>
#include <fstream>
#include <stdio.h>

//#ifdef WIN32
//    #ifndef ZLIB_WINAPI
//        #define ZLIB_WINAPI
//    #endif
//#endif

#include "zlib.h"

#include "GPUTypes.h"

/*************************************************
A D3DPixRunReader implements the reading operations
for a Pixrun file
*************************************************/
class D3D9PixRunReader
{
public:
    /*******************************************
    CID = Command IDentifier
    *******************************************/
    typedef u32bit CID;


    /*******************************************
    Open a PixRun file
    *******************************************/
    void open(std::string filename);

    /*******************************************
    Closes opened PixRun file or does nothing
    *******************************************/
    void close();

    /*******************************************
    Reads next call, return false if there
    aren't more calls.
    *******************************************/
    bool fetch();

    /*******************************************
    Returns fetched call id.

    Pre: Last fetch returned true
    *******************************************/
    CID getFetchedCID();

    /*******************************************
    Returns event id

    Pre: Last fetch returned true
    *******************************************/
    u32bit getFetchedEID();

    bool isPreloadCall();
    
    /*******************************************
    Methods for reading parameters.
    *******************************************/

    /*******************************************
    Templated method that reads a 'fixed size'
    parameter, this includes primitive types, enums, structs ...
    *******************************************/
    template < typename T > void readParameter(T *param);

    /*******************************************
    Specialization for reading the locked
    memory region that follows unlock operations.
    *******************************************/
    void readParameter(D3DBuffer *param);

    D3D9PixRunReader();


private:
    // PIXRun specific structures and id's , see
    // "PixRunFileFormat.pdf" in doc
    struct Chunk
    {
        u32bit size;
        u32bit type;
    };

    struct EventData
    {
        u32bit EDID;
        u32bit EID;
    };
    
    struct AsyncAttribute
    {
        u32bit EID;
        u32bit ADID;
    };

    struct PackedCallPackage
    {
        u32bit size;
        u32bit CID;
        u32bit one;
    };

    // Attributes
    PackedCallPackage fetchedPCP;
    u32bit fetchedEID;
    bool isD3DCALL_SYNC;
    //FILE *input;
    gzFile input;
    // std::ifstream *input;

    //std::ofstream log;
    
    //  Some statistics.
    //u64bit readData;
    //u64bit usedData;
    //u64bit skippedData;
    
    // Try to read next CID, return true if success
    bool readCID();

    // Controls if we are reading parameters
    // and have to skip before reading next CID
    bool mustSkipParameters;
    // Control of the current offset
    DWORD parameterOffset;
    // Skips the rest of parameters
    void skipParameters();

    // Read template method for structures of the PixRun files,
    // it doesn't increment parameter offset.
    template < typename T > void readPixRunStructure(T *s);

    // Skip bytes
    void skip(u32bit bytes);

};

template < typename T > void D3D9PixRunReader::readParameter(T *param)
{
    //fread((void *)param, sizeof(T), 1, input);
    gzread(input, (void *)param, sizeof(T));
    // Advance parameter offset
    parameterOffset += sizeof(T);
    //readData +=sizeof(T);
    //usedData +=sizeof(T);
    //log << "readParameter -> read " << sizeof(T) << " bytes | current file position is " << gztell(input);
    //log << " parameterOffset = " << parameterOffset;
    //if (sizeof(T) >= 4)
    //    log << " | value = " << (*(int *) param);
    //log << std::endl;
}

template < typename T > void D3D9PixRunReader::readPixRunStructure(T *param)
{
    //fread((void *)param, sizeof(T), 1, input);
    gzread(input, (void *)param, sizeof(T));
    //readData +=sizeof(T);
    //log << "readPixStructure -> read " << sizeof(T) << " bytes.  Current file position is " << gztell(input) << std::endl;
}

#endif
