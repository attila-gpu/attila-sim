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

#include "MemoryTraceRecorder.h"
#include "MemoryTransaction.h"

using namespace std;
using namespace gpu3d;
using namespace gpu3d::memorycontroller;

bool MemoryTraceRecorder::open(const char* path)
{
    trace.open(path, ios::out | ios::binary);
    if ( !trace ) {
        return false;
    }
    return true; // memory trace file properly opened
}

void MemoryTraceRecorder::record( u64bit cycle, GPUUnit clientUnit, u32bit clientSubUnit, MemTransCom memoryCommand,
                                  u32bit address, u32bit size)
{

    trace << cycle << ";" << MemoryTransaction::getBusName(clientUnit) << "[" << clientSubUnit << "];";
    switch ( memoryCommand ) {
        case MT_READ_REQ:
            trace << "MT_READ_REQ;";
            break;
        case MT_READ_DATA:
            trace << "MT_READ_DATA;";
            break;
        case MT_WRITE_DATA:
            trace << "MT_WRITE_DATA;";
            break;
        case MT_PRELOAD_DATA:
            trace << "MT_PRELOAD_DATA;";
            break;
        case MT_STATE:
            trace << "MT_STATE;";
            break;
        default:
            trace << static_cast<u32bit>(memoryCommand) << ";";
    } 
    trace << std::hex << address << std::dec << ";" << size << "\n";
}

