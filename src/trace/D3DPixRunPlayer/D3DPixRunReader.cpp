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

#include "stdafx.h"

#include "D3DBuffer.h"
#include "D3DPixRunReader.h"

#ifdef WIN32

/**
 * STL file functions in windows have can't manage
 * files >2GB The solution adopted can be found at:
 * http://www.codeproject.com/file/64-bit_fileio.asp
 **/
 extern "C" int __cdecl _fseeki64(FILE *, __int64, int);

#endif //WIN32

using namespace std;

void D3D9PixRunReader::open(string filename)
{

//#if defined(WIN32) || defined(_CYGWIN_) 
//    input = fopen(filename.c_str(), "rb");
//#else
//    input = fopen64(filename.c_str(), "rb");
//#endif //WIN32

    input = gzopen(filename.c_str(), "rb");

    if (input == 0)
        panic("D3D9PixRunReader", "open", "Error opening PIX trace file.");
    
    isD3DCALL_SYNC = false;

    //log.open("D3DPixRunReader.log");
    
    //readData = 0;
    //usedData = 0;
    //skippedData = 0;
}

void D3D9PixRunReader::close()
{
    //log << "Sumary : " << endl;
    //log << " Read Data = " << readData << " bytes | Used Data = " << usedData << " bytes | Skipped Data = " << skippedData << " bytes" << endl;
    //log.close();
        
    if (input != 0)
        gzclose(input);
}

bool D3D9PixRunReader::fetch()
{
    // Perhaps we have readed some parameter
    // from previous call
    if (mustSkipParameters)
    {
        mustSkipParameters = false;
        skipParameters();
    }

    // Read next CID
    return readCID();
}

bool D3D9PixRunReader::readCID()
{

    // Type identifiers
    const u32bit EVENT_CHUNK = 0x3EB;
    const u32bit ASYNC_ATTRIBUTE_CHUNK = 0x3EC;
    const u32bit D3DCALL_SYNC_EDID = 0x0B;
    const u32bit PACKED_CALL_PACKAGE_ADID = 19;

    // For brevity
    const u32bit sizeUL = sizeof(u32bit);

    // Loop until a packed call package is found
    bool found = false;

    Chunk c;
    
    // Read first chunk
    readPixRunStructure(&c);

    //while(!feof(input) && !found)
    while(!gzeof(input) && !found)
    {
        // Only interested in this type of chunk
        if (c.type == ASYNC_ATTRIBUTE_CHUNK)
        {
            AsyncAttribute aa;
            readPixRunStructure(&aa);

            // Only interested in this type of async attribute
            if(aa.ADID == PACKED_CALL_PACKAGE_ADID)
            {
                // Store event id
                fetchedEID = aa.EID;
                // Reached a packed call package :)
                readPixRunStructure(&fetchedPCP);
                found = true;
                
                isD3DCALL_SYNC = false;
                
                //usedData += sizeof(c) + sizeof(aa) + sizeof(fetchedPCP);
            }
            // Skip rest of chunk
            else skip(c.size - 3 * sizeUL);
        }
        else if (c.type == EVENT_CHUNK)
        {
            EventData eventData;
            readPixRunStructure(&eventData);
            
            //  Only interested on D3D Call (sync).
            if (eventData.EDID == D3DCALL_SYNC_EDID)
            {
                //  Skip the first two fields of the the event.
                skip(3 * sizeUL);
                fetchedEID = eventData.EID;
                
                // Reached a packed call package :)
                readPixRunStructure(&fetchedPCP);
                found = true;

                isD3DCALL_SYNC = true;
            }
            else
            {
                skip(c.size - 3 * sizeUL);
            }
        }
        else 
        {
            // Skip rest of chunk
            skip(c.size - sizeUL);
        }
        
        if (!found)
        {
            // Read next chunk
            readPixRunStructure(&c);
        }
    }

    // If founded we have to skip parameters
    // before read next CID
    if (found)
    {
        mustSkipParameters = true;
    }

    return found;
}

void D3D9PixRunReader::skipParameters()
{
    //log << "skipParameters -> fetchedPCP.size = " << fetchedPCP.size << " parameterOffset = " << parameterOffset << endl;
    
    //skippedData += fetchedPCP.size - sizeof(DWORD) * 2 - parameterOffset;

    // We have already readed some fields of PackedCallPackage
    skip(fetchedPCP.size - sizeof(DWORD) * 2 - parameterOffset);
    parameterOffset = 0;
}

D3D9PixRunReader::CID D3D9PixRunReader::getFetchedCID()
{
    //log << "getFetchedCID -> CID = " << fetchedPCP.CID << endl;
    
    return fetchedPCP.CID;
}

u32bit D3D9PixRunReader::getFetchedEID() {
    return fetchedEID;
}

bool D3D9PixRunReader::isPreloadCall()
{
    return isD3DCALL_SYNC;
}

D3D9PixRunReader::D3D9PixRunReader(): mustSkipParameters(false),
parameterOffset(0), fetchedEID(0), input(0)
{
//    cout << "Sizes: " << endl;
//    cout << " INT " << sizeof(INT) << endl;
//    cout << " UINT " << sizeof(UINT) << endl;
//    cout << " BOOL " << sizeof(BOOL) << endl;
//    cout << " HRESULT " << sizeof(HRESULT) << endl;
//    cout << " DWORD " << sizeof(DWORD) << endl;
//    cout << " ULONG " << sizeof(ULONG) << endl;
//    cout << " LONG " << sizeof(LONG) << endl;
//    cout << " BYTE " << sizeof(BYTE) << endl;
//    cout << " WORD " << sizeof(WORD) << endl;
//    cout << " FLOAT " << sizeof(FLOAT) << endl;
//    cout << " HWND " << sizeof(HWND) << endl;
//    cout << " HDC " << sizeof(HDC) << endl;
//    cout << " HMONITOR " << sizeof(HMONITOR) << endl;
//    cout << " void* " << sizeof(void*) << endl;
}

void D3D9PixRunReader::readParameter(D3DBuffer *param)
{
    // skip buffer address (UNUSED for now)
    //void *address;
    u32bit address;
    readParameter(&address);

    // Read buffer size
    DWORD size;
    readParameter(&size);

    // skip buffer zero field
    DWORD zero;
    readParameter(&zero);

    // skip buffer size 2 field
    DWORD size2;
    readParameter(&size2);

    // Allocate enough memory
    param->setSize(size);

    // Access to memory
    void *p = 0;
    param->lock(&p);
        // Store readed param
        //fread((void *)p, size, 1, input);
        gzread(input, (void *) p, size);
        //readData += size;
        //usedData += size;
    param->unlock();

    // Advance offset
    parameterOffset += size;    

    //log << "readParameter (D3DBuffer) -> read " << size << " bytes.  Current file position is " << gztell(input);
    //log << " parameterOffset = " << parameterOffset << endl;
}

void D3D9PixRunReader::skip(u32bit offset)
{
    //log << "skip -> skipped " << offset << " bytes.  Current file position is " << gztell(input) << endl;
    
//#ifdef WIN32
//    _fseeki64(input, offset, SEEK_CUR );
//#else
//    fseek(input, offset, SEEK_CUR);
//#endif //WIN32
    gzseek(input, offset, SEEK_CUR);
}
