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
 * $RCSfile: RegisterWriteBufferAGP.cpp,v $
 * $Revision: 1.2 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:57 $
 *
 * RegisterWriteBufferAGP implementation file
 *
 */
 
 
/**
 *
 *  @file RegisterWriteBufferAGP.cpp
 *
 *  This file contains the implementation of a GPU register write buffer class used by the extractTraceRegion tool to
 *  store register writes until the start of the region to extract from the original AGP transaction trace.
 *
 */

#include "RegisterWriteBufferAGP.h"
#include <iostream>

using namespace std;
using namespace gpu3d;

RegisterWriteBufferAGP::RegisterWriteBufferAGP()
{}

void RegisterWriteBufferAGP::writeRegister(GPURegister reg, u32bit index, const GPURegData& data, u32bit md)
{
    RegisterIdentifier ri(reg, index);
    WriteBufferIt it = writeBuffer.find(ri);
    if ( it != writeBuffer.end() ) // register found, update contents
    {
        it->second.data = data;
        it->second.md = md;
    }
    else // register not written yet, create a new entry
    {
        RegisterData rd(data, md);
        writeBuffer.insert(make_pair(ri, rd));
    }
    registerWritesCount++;
}

bool RegisterWriteBufferAGP::readRegister(GPURegister reg, u32bit index, GPURegData& data, u32bit &md)
{
    RegisterIdentifier ri(reg, index);
    WriteBufferIt it = writeBuffer.find(ri);
    if ( it != writeBuffer.end() ) // register found, update contents
    {
        data = it->second.data;
        md = it->second.md;
        return true;
    }
    else
    {
        // register not written yet, create a new entry
        return false;    
    }
}

bool RegisterWriteBufferAGP::flushNextRegister(gpu3d::GPURegister &reg, u32bit &index, gpu3d::GPURegData &data, u32bit &md)
{
    //cout << "RegisterWriteBuffer::flushNextRegister() -> Doing pendent register writes: " << writeBuffer.size() << " (saved "
    //<< registerWritesCount - writeBuffer.size() << " reg. writes)" << endl;

    //  GPU_INDEX_STREAM must be sent before the configuration for the stream.
    //    But the storing order in the map is the inverse.

    RegisterIdentifier ri(GPU_INDEX_STREAM, 0);
    WriteBufferIt it = writeBuffer.find(ri);

    ///  Check if GPU_INDEX_STREAM was found.
    if ( it != writeBuffer.end() ) // register found, update contents
    {
        //  Get the register data.  */
        reg = GPU_INDEX_STREAM;
        index = 0;
        data = it->second.data;
        md = 0;

        //  Remove the register update from the write buffer.
        writeBuffer.erase(it);
        
        //  Update stored register writes counter.
        registerWritesCount--;

        //  A register was extracted from the buffer.
        return true;
    }
    else
    {
        // Read next ATTILA register
        WriteBufferIt it = writeBuffer.begin();
        
        //  Check if all registers have been extracted.
        if(it != writeBuffer.end())
        {
            //  Get the register data.
            reg = it->first.reg;
            index = it->first.index;
            data = it->second.data;
            md = it->second.md;

            //  Remove the register update from the write buffer.
            writeBuffer.erase(it);

            //  Update stored register writes counter.
            registerWritesCount--;

            //  A register was extracted from the buffer.
            return true;
        }
        else
        {
            //  No register writes remaining in the buffer.
            return false;
        }
    }
}

