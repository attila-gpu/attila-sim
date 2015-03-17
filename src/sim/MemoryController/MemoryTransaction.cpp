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

#include "MemoryTransaction.h"
#include "MemorySpace.h"
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <cstring>

using namespace gpu3d;
using std::string;
using namespace std;

u32bit MemoryTransaction::instances = 0;

u32bit MemoryTransaction::countInstances()
{
    return instances;
}

u32bit MemoryTransaction::busWidth[LASTGPUBUS] =
{
    8, // Data bus with the Command Processor
    16, // Data bus with the Streamer Fetch unit (indexes)
    16, // Data bus with the Streamer Loader unit (attributes)
    32, // Data bus with the Z Stencil Test unit (pixels depth/stencil)
    32, // Data bus with the Color Write unit (pixels color)
    8, // Data bus with the DAC unit (pixels color)
    16, // Data bus with the Texture Unit
#ifndef USE_MEMORY_CONTROLLER_V2
   16, // Data bus with the Memory modules
#endif
    8 // Data bus with system memory
};

const char* MemoryTransaction::busNames[LASTGPUBUS] =
{
    "CommandProcessor",
    "StreamerFetch",
    "StreamerLoader",
    "ZStencilTest",
    "ColorWrite",
    "DAC",
    "TextureUnit",
#ifndef USE_MEMORY_CONTROLLER_V2
   "MemoryModule",
#endif
    "System"
};

//////////////////////////////////////////////////////////////////
/// STATIC METHODS TO QUERY AND CHANGE BUS WIDTH AND BUS NAMES ///
//////////////////////////////////////////////////////////////////
const char* MemoryTransaction::getBusName(GPUUnit unit)
{
    if ( unit >= LASTGPUBUS )
        panic("MemoryTransaction", "getBusName", "Unknown GPU Unit");
    return busNames[unit];
}

u32bit MemoryTransaction::getBusWidth(GPUUnit unit)
{
    if ( unit >= LASTGPUBUS )
        panic("MemoryTransaction", "getBusWidth", "Unknown GPU Unit");
    return busWidth[unit];
}
void MemoryTransaction::setBusWidth(GPUUnit unit, u32bit newBW)
{
    if ( unit >= LASTGPUBUS )
        panic("MemoryTransaction", "setBusWidth", "Unknown GPU Unit");
    if ( newBW < 8 || newBW > 1024 )
        panic("MemoryTransaction", "setBusWidth",
              "Bandwidth must be within the range [8,1024]");
    busWidth[unit] = newBW;
}

////////////////////////////////
/// Memory Transaction CTORS ///
////////////////////////////////

// Memory transaction operation ctor
// (for MT_READ_DATA, MT_READ_REQ, MT_WRITE_DATA, MT_PRELOAD_DATA)
MemoryTransaction::MemoryTransaction(MemTransCom com, u32bit addr,
    u32bit sz, u8bit *dataBuffer, GPUUnit source, u32bit id) :
    command(com), address(addr), size(sz), sourceUnit(source), unitID(0), ID(id),
    state(MS_BOTH)
{
    ++instances;

    // Check transaction consistency
    GPU_ASSERT
    (
        if ((com != MT_READ_REQ) && (com != MT_READ_DATA)
            && (com != MT_WRITE_DATA) && (com != MT_PRELOAD_DATA))
            panic("MemoryTransaction", "MemoryTransaction",
                 "Incorrect memory transaction operation");
       if ( (com == MT_PRELOAD_DATA) && (source != COMMANDPROCESSOR))
            panic("MemoryTransaction", "MemoryTransaction",
                  "PRELOAD transaction can only be used by Command Processor");
        if ( dataBuffer == 0 )
            panic("MemoryTransaction", "MemoryTransaction", "NULL pointer not allowed");
        if ( size == 0 )
            panic("MemoryTransaction", "MemoryTransaction",
                  "Memory transaction with size 0 is not allowed.");
        if ((size > MAX_TRANSACTION_SIZE) && (com != MT_PRELOAD_DATA))
            panic("MemoryTransaction", "MemoryTransaction",
                  "Memory transaction size exceeds maximum transaction size.");
    )

    masked = false;
    switch(com)
    {
        case MT_READ_REQ:
            // Copy the pointer to the buffer where to store the data
            readData = dataBuffer;
            //  Spends a single bus cycles.
         // (it is not currently accounted as a separate address/request buffer)
            cycles = 1;
            //  Set the transaction info field
            sprintf((char *) getInfo(), "READ_REQ @%x, %d", addr, size);
            break;
        case MT_READ_DATA:
            // Copy the pointer to the buffer where to store the data
            readData = dataBuffer;
            //  Calculate transaction bus cycles
            cycles = (u32bit) ceil((f32bit) size / (f32bit) busWidth[sourceUnit]);
            //  Set the transaction info field
            sprintf((char *) getInfo(), "READ_DATA @%x, %d", addr, size);
            break;
        case MT_WRITE_DATA:
            //  Copy data from the input buffer to the write data buffer
            memcpy(writeData, dataBuffer, size);
            //  Calculate transaction bus cycles
            cycles = (u32bit) ceil((f32bit) size / (f32bit) busWidth[sourceUnit]);
            // Set the transaction info field
            sprintf((char *) getInfo(), "WRITE_DATA @%x, %d", addr, size);
            break;
        case MT_PRELOAD_DATA:
            //  Copy data from the input buffer to the write data buffer
            preloadData = dataBuffer;
            //  Calculate transaction bus cycles
            cycles = 0; // instantaneous copy of any amount of bytes
            //  Set the transaction info field
            sprintf((char *) getInfo(), "PRELOAD_DATA @%x, %d", addr, size);
            break;
        default:
            panic("MemoryTransaction", "MemoryTransaction",
                 "Unsupported memory transaction type.");
    }

    //  Set object color for tracing based on command
    setColor(command);
    setTag("MemTr");
/*
 if ( com == MT_PRELOAD_DATA && id == 20 )
    {
        cout << "addr = " << addr << "\n";
        cout << "sz = " << sz << "\n";
        cout << "*dataBuffer = " << (u32bit)dataBuffer << "\n";
        cout << "SourceUnit = " << sourceUnit << "\n";
        cout << "UnitID = " << unitID << "\n";
        cout << "ID = " << id << "\n";
        cout << "State = " << state << "\n";
        cout << "Cycles = " << cycles << "\n";
    }


  if ( com == MT_PRELOAD_DATA )
      cout << "HC: " << getHashCode(false) << " - HCd: " << getHashCode(true) << "\n";
       */
}


//  Memory transaction operation for MASKED MT_WRITE_DATA
MemoryTransaction::MemoryTransaction(u32bit addr, u32bit sz, u8bit *dataBuffer,
    u32bit *writeMask, GPUUnit source, u32bit id) :
    address(addr), size(sz), sourceUnit(source), unitID(0), ID(id), state(MS_BOTH)
{
    ++instances;

    // Set as MT_WRITE_DATA transaction
    command = MT_WRITE_DATA;

    GPU_ASSERT
    (
        if (dataBuffer == 0)
            panic("MemoryTransaction", "MemoryTransaction", "NULL pointer not allowed");
        if (writeMask == 0)
            panic("MemoryTransaction", "MemoryTransaction", "NULL pointer not allowed");
        if (size == 0)
            panic("MemoryTransaction", "MemoryTransaction",
                 "Memory transaction with size 0 is not allowed");
        if (size > MAX_TRANSACTION_SIZE)
            panic("MemoryTransaction", "MemoryTransaction",
                "Memory transaction size exceed maximum transaction size");
    )

    masked = true;
    memcpy(writeData, dataBuffer, size); // Copy the data into the internal buffer
    memcpy(mask, writeMask, WRITE_MASK_SIZE * sizeof(u32bit)); // Copy the write mask
    // Calculate transaction bus cycles
    cycles = (u32bit) ceil((f32bit) size / (f32bit) busWidth[sourceUnit]);
    // Set the transaction info field
    sprintf((char *) getInfo(), "WRITE_DATA(masked) @%x, %d", addr, size);
    // Set object color for tracing
    setColor(command);
    setTag("MemTr");
}

//  Memory transaction operation (for MT_READ_DATA, MT_READ_REQ and MT_WRITE_DATA).
//  For units supporting REPLICATION
MemoryTransaction::MemoryTransaction(MemTransCom com, u32bit addr,
    u32bit sz, u8bit *dataBuffer, GPUUnit source, u32bit sourceID, u32bit id) :
    unitID(sourceID), command(com), address(addr), size(sz), sourceUnit(source), ID(id),
    state(MS_BOTH)
{
    ++instances;

    // check transaction consistency
    GPU_ASSERT
    (
        if ((com != MT_READ_REQ) && (com != MT_READ_DATA) && (com != MT_WRITE_DATA))
            panic("MemoryTransaction", "MemoryTransaction",
                  "Incorrect memory transaction operation");
        if (dataBuffer == 0)
            panic("MemoryTransaction", "MemoryTransaction", "NULL pointer not allowed");
        if (size == 0)
            panic("MemoryTransaction", "MemoryTransaction",
                  "Memory transaction with size 0 is not allowed");
        if (size > MAX_TRANSACTION_SIZE)
            panic("MemoryTransaction", "MemoryTransaction",
                  "Memory transaction size exceeds maximum transaction size");
    )

    masked = false; // this write transaction is not masked

    switch(com)
    {
        case MT_READ_REQ:
            // Copy the pointer to the buffer where to store the data
            readData = dataBuffer;
            // Spends a single bus cycles.
            // (It is not currently accounted as a separate address/request buffer)
            cycles = 1;
            // Set the transaction info field
            sprintf((char *) getInfo(), "READ_REQ @%x, %d", addr, size);
            break;
        case MT_READ_DATA:
            // Copy the pointer to the buffer where to store the data
            readData = dataBuffer;
            // Calculate transaction bus cycles
            cycles = (u32bit) ceil((f32bit) size / (f32bit) busWidth[sourceUnit]);
            // Set the transaction info field
            sprintf((char *) getInfo(), "READ_DATA @%x, %d", addr, size);
            break;
        case MT_WRITE_DATA:
            // Copy data from the input buffer to the write data buffer
            memcpy(writeData, dataBuffer, size);
            // Calculate transaction bus cycles
            cycles = (u32bit) ceil((f32bit) size / (f32bit) busWidth[sourceUnit]);
            // Set the transaction info field.  */
            sprintf((char *) getInfo(), "WRITE_DATA @%x, %d", addr, size);
            break;
        default:
            panic("MemoryTransaction", "MemoryTransaction",
                  "Unsupported memory transaction type.");
            break;
    }
    // Set object color for tracing based on command type
    setColor(command);
    setTag("memTr");
}

//Memory transaction operation (for masked MT_WRITE_DATA)
MemoryTransaction::MemoryTransaction(u32bit addr, u32bit sz, u8bit *dataBuffer,
    u32bit *writeMask, GPUUnit source, u32bit sourceID, u32bit id) :
    unitID(sourceID),
    address(addr), size(sz), sourceUnit(source), ID(id), state(MS_BOTH)
{
    ++instances;

    //Set as MT_WRITE_DATA transaction
    command = MT_WRITE_DATA;

    // Check transaction consistency
    GPU_ASSERT
    (
        if (dataBuffer == NULL)
            panic("MemoryTransaction", "MemoryTransaction", "NULL pointer not allowed");
        if (writeMask == 0)
            panic("MemoryTransaction", "MemoryTransaction", "NULL pointer not allowed");
        if (size == 0)
            panic("MemoryTransaction", "MemoryTransaction",
                  "Memory transaction with size 0 is not allowed");
        if (size > MAX_TRANSACTION_SIZE)
            panic("MemoryTransaction", "MemoryTransaction",
                  "Memory transaction size exceeds maximum transaction size");
    )

    // Set as a masked write transaction
    masked = TRUE;
    //Copy the data to write to the internal buffer.  */
    memcpy(writeData, dataBuffer, size);
    // Copy the write mask.  */
    memcpy(mask, writeMask, WRITE_MASK_SIZE * sizeof(u32bit));
    // Calculate transaction bus cycles
    cycles = (u32bit) ceil((f32bit) size / (f32bit) busWidth[sourceUnit]);
    //Set the transaction info field
    sprintf((char *) getInfo(), "WRITE_DATA(masked) @%x, %d", addr, size);
    //Set object color for tracing
    setColor(command);
    setTag("memTr");
}

// Constructor for MT_READ_DATA
MemoryTransaction::MemoryTransaction(MemoryTransaction *request)
{
    ++instances;

    //  Check original transaction is a read request
    GPU_ASSERT
    (
        if (request->command != MT_READ_REQ)
            panic("MemoryTransaction", "MemoryTransaction", "Illegal source transaction.");
    )

    //  Set transaction operation
    command = MT_READ_DATA;

    // Set new transaction parameters
    address = request->address;
    size = request->size;
    readData = request->readData;
    sourceUnit = request->sourceUnit;
    ID = request->ID;
    unitID= request->unitID;
    masked = FALSE;
    state = request->state;
    // Calculate number of transaction packets
    cycles = (u32bit) ceil((f32bit) size / (f32bit) busWidth[sourceUnit]);
    // Set object color for tracing
    setColor(command);
    // Copy cookies from the original transaction (STV feedback)
    copyParentCookies(*request);
    // Copy info field from the original transaction
    strcpy((char *) getInfo(), (char *) request->getInfo());
    setTag("memTr");
}

MemoryTransaction::MemoryTransaction(MemState memState) : state(memState)
{
    ++instances;

    command = MT_STATE; // set transaction operation
    cycles = 1; // Only one cycle for controller state transactions
    setColor(command+memState); // set color for tracing
    setTag("memTr"); // string for tracing
}


MemoryTransaction* MemoryTransaction::createReadData(MemoryTransaction* reqDataTran)
{
    return new MemoryTransaction(reqDataTran);
}

MemoryTransaction* MemoryTransaction::createStateTransaction(MemState state)
{
    return new MemoryTransaction(state);
}

////////////////////////////////////////
/// Memory Transaction QUERY METHODS ///
////////////////////////////////////////

MemTransCom MemoryTransaction::getCommand() const
{
    return command;
}

u32bit MemoryTransaction::getAddress() const
{
    return address;
}

u32bit MemoryTransaction::getSize() const
{
    return size;
}

u8bit *MemoryTransaction::getData() const
{
    switch(command)
    {
        case MT_READ_REQ:
        case MT_READ_DATA:
            return readData;
            break;
        case MT_WRITE_DATA:
            return (u8bit *)writeData;
            break;
        case MT_PRELOAD_DATA:
            return preloadData;
            break;
        default:
            panic("MemoryTransaction", "getData", "Unsupported transaction type.");
            break;
    }
    return 0;
}

GPUUnit MemoryTransaction::getRequestSource() const
{
    return sourceUnit;
}

u32bit MemoryTransaction::getID() const
{
    return ID;
}

u32bit MemoryTransaction::getBusCycles() const
{
    return cycles;
}

MemState MemoryTransaction::getState() const
{
    return state;
}

bool MemoryTransaction::isMasked() const
{
    return masked;
}

u32bit* MemoryTransaction::getMask() const
{
    return (u32bit*)mask;
}

u32bit MemoryTransaction::getUnitID() const
{
    return unitID;
}

void MemoryTransaction::setRequestID(u32bit id)
{
    requestID = id;
}

u32bit MemoryTransaction::getRequestID() const
{
    return requestID;
}

string MemoryTransaction::toString(bool compact) const
{
    std::stringstream ss;

    if ( compact ) {
        ss << getRequestSourceStr(true) << "[" << getUnitID() << "] ";
        string cmd;
        switch ( command ) 
        {
            case MT_READ_REQ:
                ss << "READ_REQ"; break;
            case MT_WRITE_DATA:
                ss << "WRITE_DATA"; break;
            case MT_READ_DATA:
                ss << "READ_DATA"; break;
            case MT_PRELOAD_DATA:
                ss << "PRELOAD_DATA"; break;
            case MT_STATE:
                ss << "MT_STATE"; break;
                ss << " (";
                switch ( state ) 
                {
                    case MS_NONE:
                        ss << "NONE)"; break;
                    case MS_READ_ACCEPT:
                        ss << "READ_ACCEPT)"; break;
                    case MS_WRITE_ACCEPT:
                        ss << "WRITE_ACCEPT)"; break;
                    case MS_BOTH:
                        ss << "BOTH)"; break;
                }
                return ss.str(); // nothing else to be processed for a memory state transaction
        }

        ss << " id=" << getID();
        ss << " addr=" << hex  << address << dec;
        ss << " (" << size << "B)";
        ss << " C=" << cycles;

        if ( masked )
            ss << " (masked)";

        return ss.str();
    }

    ss << "Source: " << getRequestSourceStr() << " unit: " << getUnitID()
        << " ID: " << getID()
        << " Type: ";
    switch ( command )
    {
        case MT_READ_REQ:
            ss << "MT_READ_REQ"; break;
        case MT_WRITE_DATA:
            ss << "MT_WRITE_DATA"; break;
        case MT_READ_DATA:
            ss << "MT_READ_DATA"; break;
        case MT_PRELOAD_DATA:
            ss << "MT_PRELOAD_DATA"; break;
        case MT_STATE:
            ss << "MT_STATE"; break;
            ss << " state=";
            switch ( state ) {
                case MS_NONE:
                    ss << "NONE"; break;
                case MS_READ_ACCEPT:
                    ss << "READ_ACCEPT"; break;
                case MS_WRITE_ACCEPT:
                    ss << "WRITE_ACCEPT"; break;
                case MS_BOTH:
                    ss << "BOTH"; break;
            }
            return ss.str(); // nothing else to be processed for a memory state transaction
    }
    if ( masked )
        ss << " -masked- ";

    ss << "(" << size << ") ";
    ss << " Address: " << hex  << address << dec;
    ss << " cycles: " << cycles;

    return ss.str();
}

void MemoryTransaction::dump(bool dumpData) const
{
    using namespace std;

    string info = toString(); // string me!

    cout << info;

    const u8bit* ptr = 0;
    if ( command == MT_READ_DATA )
        ptr = readData;
    else if ( command == MT_WRITE_DATA )
        ptr = writeData;
    else if ( command == MT_PRELOAD_DATA )
        ptr = preloadData;

    if ( ptr && dumpData )
    {
        cout << " Data: ";
        for ( u32bit i = 0; i < size; i++ )
            cout << hex << (u32bit)ptr[i] << dec << ".";
    }

}

std::string MemoryTransaction::getRequestSourceStr(bool compactName) const
{
    switch ( sourceUnit )
    {
        case COMMANDPROCESSOR:
            if ( compactName )
                return string("CP");
            else
                return string("COMMANDPROCESSOR");
        case STREAMERFETCH:
            if ( compactName )
                return string("SF");
            else
                return string("STREAMERFETCH");
        case STREAMERLOADER:
            if ( compactName )
                return string("SF");
            else
                return string("STREAMERLOADER");
        case DACB:
            if ( compactName )
                return string("DAC");
            else
                return string("DACB");
        case TEXTUREUNIT:
            if ( compactName )
                return string("TXT");
            else
                return string("TEXTUREUNIT");
        case ZSTENCILTEST:
            if ( compactName )
                return string("Z");
            else
                return string("ZSTENCILTEST");
        case COLORWRITE:
            if ( compactName )
                return string("COL");
            else
                return string("COLORWRITE");
        case MEMORYMODULE:
            if ( compactName )
                return string("MEM");
            else
                return string("MEMORYMODULE");
        case SYSTEM:
            if ( compactName )
                return string("SYS");
            else
                return string("SYSTEM");
        default:
            return string("UNKNOWN");
    }
}

string MemoryTransaction::getIdentificationStr(bool concatData) const
{
    std::stringstream ss;

    ss  << command << "-"
        << state << "-"
        << address << "-"
        << size << "-"
        << masked << "-"
        << cycles << "-"
        << sourceUnit << "-"
        << unitID << "-";
        //<< ID << "-"; // Id's on replicated unit are shared (excluded to debug)
        //<< requestID << "-"; // Excluded from identification string

    const u8bit* ptr = 0;
    if ( command == MT_READ_DATA )
        ptr = readData;
    else if ( command == MT_WRITE_DATA )
        ptr = writeData;
    else if ( command == MT_PRELOAD_DATA )
        ptr = preloadData;

    // concatenate data
    if ( ptr && concatData)
    {
        for ( u32bit i = 0; i < size; i++ )
        {
            ss << hex << (u32bit)ptr[i] << dec << ".";
        }
    }
    return ss.str();
}

u32bit MemoryTransaction::getHashCode(bool useData) const
{
    u32bit sum = 0;
    string s = getIdentificationStr(useData);
    string::const_iterator it = s.begin();
    for ( u32bit i = 1; it != s.end(); it++, i++ )
        sum += (((u32bit)*it) * i) % 255;
    return sum;
}

bool MemoryTransaction::isToSystemMemory() const
{
    return ((address & ADDRESS_SPACE_MASK) == SYSTEM_ADDRESS_SPACE);
}


MemoryTransaction::~MemoryTransaction()
{
    --instances;
}
