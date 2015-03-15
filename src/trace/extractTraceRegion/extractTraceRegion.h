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
 * $RCSfile: extractTraceRegion.h,v $
 * $Revision: 1.2 $
 * $Author: vmoya $
 * $Date: 2007-01-07 17:35:33 $
 *
 * extractTraceRegion definition file
 *
 */
 
 
/**
 *
 *  @file extractTraceRegion.h
 *
 *  This file contains definitions the tool that extracts AGP transactions for a frame region of
 *  the input tracefile.
 *
 */
 
#ifndef _EXTRACTTRACEREGION_

#define _EXTRACTTRACEREGION_

#include "GPUTypes.h"
#include "zfstream.h"
#include "AGPTransaction.h" 

struct UploadRegionIdentifier
{
    u32bit startAddress;
    u32bit endAddress;

    UploadRegionIdentifier(u32bit start, u32bit end) :
        startAddress(start), endAddress(end)
    {}
    
    friend bool operator<(const UploadRegionIdentifier& lv,
                          const UploadRegionIdentifier& rv)
    {
        return (lv.endAddress < rv.startAddress);
    }

    friend bool operator==(const UploadRegionIdentifier& lv, 
                           const UploadRegionIdentifier& rv)
    {
        return ((lv.startAddress <= rv.startAddress) && (lv.endAddress >= rv.startAddress)) ||
               ((lv.startAddress <= rv.endAddress) && (lv.endAddress >= rv.endAddress)) || 
               ((lv.startAddress >= rv.startAddress) && (lv.startAddress <= rv.endAddress));
    }
};

/**
 *
 *  This structure holds information about a GPU memory region where data has been uploaded
 *  by an AGP Transaction.
 *
 */
 
struct UploadRegion
{
    u32bit address;         /**<  Start address of the upload region.  */
    u32bit size;            /**<  Size in bytes of the upload region.  */
    u32bit lastMD;          /**<  Last memory descriptor identifier associated with the upload region.  */
    u32bit lastAGPTransID;  /**<  Last AGP transaction identifier associated with the upload region.  */

    //  Constructor.
    UploadRegion(u32bit addr, u32bit sz, u32bit md, u32bit agpID) :
        address(addr), size(sz), lastMD(md), lastAGPTransID(agpID)
    {}
};
 

/**
 *
 *  This structure holds information about a shader program or a block of splitted code uploaded into the
 *  GPU by an AGP transaction.
 *
 */
 
struct ShaderProgram
{
    u32bit pc;                  /**<  Start PC (address in shader program memory) of the shader program or code block.  */
    u32bit address;             /**<  Address in memory from which the Shader Program was loaded.  */
    u32bit size;                /**<  Size in bytes of the shader program or code block.  */  
    u32bit instructions;        /**<  Number of instructions in the shader program or code block.  */
    u32bit lastAGPTransID;      /**<  Last AGP transaction identifier associated with the upload region.  */

    //  Size of a shader instruction in bytes.
    static const u32bit SHADERINSTRUCTIONSIZE = 16;

    //  Number of instructions that can be stored in the shader program memory
    static const u32bit MAXSHADERINSTRUCTIONS = 512;
    
    //  Constructor.
    ShaderProgram(u32bit pc_, u32bit addr, u32bit instr, u32bit agpID) :
        pc(pc_), address(addr), size(instr * SHADERINSTRUCTIONSIZE), instructions(instr), lastAGPTransID(agpID)
    {
    }
};


#define SHADER_PROGRAM_HASH_SIZE 1025

typedef map<u32bit, ShaderProgram> ShaderProgramMap;

bool checkAndCopyAGPTraceHeader(gzifstream *in, gzofstream *out, u32bit startFrame, u32bit extractFrames);
void insertUploadRegion(u32bit address, u32bit size, u32bit agpTransID, u32bit md);
bool checkUploadRegion(u32bit address, u32bit size, u32bit md, u32bit agpTransID);
void insertTouchedRegion(UploadRegion &upRegion);
bool checkRegionTouched(u32bit md);
void insertFragmentProgram();
void insertVertexProgram();
void insertShaderProgram(u32bit pc, u32bit address, u32bit size, ShaderProgramMap *shaderPrograms);
void checkFragmentProgram(gpu3d::AGPTransaction *agpTrans, u32bit agpTransID);
void checkVertexProgram(gpu3d::AGPTransaction *agpTrans, u32bit agpTransID);
void checkShaderProgram(u8bit *data, u32bit agpTransID, ShaderProgramMap *shaderPrograms, u8bit *shaderCache);
void loadFragmentShaderPrograms(gzofstream *outTrace);
void loadVertexShaderPrograms(gzofstream *outTrace);
void loadShaderPrograms(gzofstream *outTrace, u8bit *shaderProgramCache, gpu3d::GPURegister addressReg,
                        gpu3d::GPURegister pcReg, gpu3d::GPURegister sizeReg, gpu3d::GPUCommand loadCommand);



#endif

