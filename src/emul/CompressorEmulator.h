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

#ifndef COMPRESSOREMULATOR_H_
#define COMPRESSOREMULATOR_H_

#include "GPUTypes.h"

namespace gpu3d
{

class CompressorInfo
{
public:
    bool success;        // Has been compressed/decompressed 
    int level;            // Compression level
    int size;            // Block size after compression/decompression
    
    CompressorInfo() {}
    
    CompressorInfo(bool success, int level, int size)
        : success(success), level(level), size(size) { }
    
    CompressorInfo(const CompressorInfo& info)
        : success(info.success), level(info.level), size(info.size) {}
    
    /*CompressorInfo& operator= (const CompressorInfo& info) {
        success = info.success;
        level = info.level;
        size = info.size;
        return *this;
    }*/
};

class CompressorEmulator
{
public:
    CompressorEmulator() { }
    virtual ~CompressorEmulator() { }
    
    virtual CompressorInfo compress(const void *input, void *output, int size) = 0;
    
    virtual CompressorInfo uncompress(const void *input, void *output, int size, int level) = 0;
    
    virtual u32bit getLevelBlockSize(unsigned int level) = 0;
};

}
#endif /*COMPRESSOREMULATOR_H_*/
