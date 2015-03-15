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

#ifndef MEMORYTRACERECORDER_H
    #define MEMORYTRACERECORDER_H

#include "GPUTypes.h"
#include "MemoryControllerDefs.h"
#include <fstream>
// #include "zfstream.h"


namespace gpu3d 
{
namespace memorycontroller
{

class MemoryTraceRecorder
{
public:

    bool open(const char* path);

    void record( u64bit cycle, GPUUnit clientUnit, u32bit clientSubUnit, MemTransCom memoryCommand,
                 u32bit address, u32bit size);

private:

    std::ofstream trace;
    // gzofstream trace;

}; // class MemoryTraceRecorder

} // namespace memorycontroller
} // namespace gpu3d

#endif // MEMORYTRACERECORDER_H
