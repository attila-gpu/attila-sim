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
 * $RCSfile: AGPTraceFile.h,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:56 $
 *
 * AGP Trace File Definitions File
 *
 */


/**
 *
 *  @file AGPTraceFile.h
 *
 *  This file contains definitions related with the AGP transaction trace files.
 *
 */
 
#ifndef _AGPTRACEFILE_

#define _AGPTRACEFILE_

#include "GPUTypes.h"


///<  Defines the size of the header section of the AGP Transaction trace file.
static const u32bit AGPTRACEFILE_HEADER_SIZE = 16384;

///<  Defines the signature string for AGP Transaction trace files.
static const char AGPTRACEFILE_SIGNATURE[7] = "ATILAt";

///<  Defines the current version identifier for AGP Transaction trace files.
static const u32bit AGPTRACEFILE_CURRENT_VERSION = 0x0100;

/// 
// 
//  This structure defines the parameters associated with the AGP transaction trace file.
//
struct AGPTraceFileParameters
{
    
    //  Translation related parameters.
    
    u32bit startFrame;          ///<  First frame from the original OpenGL trace in the AGP transaction trace file.
    u32bit traceFrames;         ///<  Number of frames in the AGP transaction trace file (may not be accurate).
    
    //  Simulation related parameters.
    
    u32bit memSize;             ///<  Size of the simulated GPU memory (GDDR).
    u32bit mappedMemSize;       ///<  Size of the simulated system memory accessable from the GPU.
    u32bit textBlockDim;        ///<  Texture blocking first level tile size in 2^n x 2^n texels.
    u32bit textSBlockDim;       ///<  Texture blocking second level tile size in 2^n x 2^n first level tiles.
    u32bit scanWidth;           ///<  Rasterization scan tile (first level) width in pixels.
    u32bit scanHeight;          ///<  Rasterization scan tile (first level) heigh in pixels.
    u32bit overScanWidth;       ///<  Rasterization over scan tile (second level) width in pixels.
    u32bit overScanHeight;      ///<  Rasterization over scan tile (second level) height in pixels.
    bool doubleBuffer;          ///<  Flag that enabled double buffering for the color buffer.
    u32bit fetchRate;           ///<  Hint to the OpenGL library to optimize shader programs for the given number of parallel SIMD ALUs.
    bool memoryControllerV2;    ///<  Flag that when enabled tells that the second version of the Memory Controller will be used for simulation.
    bool v2SecondInterleaving;  ///<  Flag that enables the second data interleaving mode in the second version of the Memory Controller.
    
};

/**
 *
 *  This structure defines the content of the header section of an AGP Transaction trace file
 *
 */
 
struct AGPTraceFileHeader
{
    char signature[8];      ///<  Signature string for AGP transaction trace files.
    u32bit version;         ///<  Version number of the AGP transaction file.
    
    union
    {
        AGPTraceFileParameters parameters;  ///<  Trace file parameters.
        
        u8bit padding[AGPTRACEFILE_HEADER_SIZE];    ///< Padding to the header size.
    };
    
};

#endif
