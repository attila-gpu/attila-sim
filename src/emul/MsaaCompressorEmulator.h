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

#ifndef MSAACOMPRESSOREMULATOR_H_
#define MSAACOMPRESSOREMULATOR_H_

#include "GPUTypes.h"
#include "CompressorEmulator.h"

namespace gpu3d
{

/* This class represents the configuration used to compress a block. */

class MsaaLevelConfig {
public:
    int numRefs;    /* Number of references used for each subblock. */
    int samples;    /* Number of samples used for each subblock. */
    int size;        /* The compressed block size (if compression success). */
    
    MsaaLevelConfig() {}

    MsaaLevelConfig(int numRefs, int samples, int size) 
        : numRefs(numRefs), samples(samples), size(size) {}
};

/* This class represents the references found in a block. */

class MsaaRefs {
public:
    u32bit ref1;    /* first reference */
    u32bit ref2;    /* second reference (if using 2 references) */
    u32bit mask;    /* references index mask */

    int samples;    /* number of samples used in the subblock */

    MsaaRefs() {}

    MsaaRefs(u32bit ref1, u32bit ref2, u32bit mask) 
        : ref1(ref1), ref2(ref2), mask(mask), samples(0) {}
};

/* This class implements the msaa compression algorithm. */

class MsaaCompressorEmulator : public CompressorEmulator
{
public:
    /** blockSize     number of elements for each block */
    MsaaCompressorEmulator(/*int numLevels,*/ int blockSize);
    virtual ~MsaaCompressorEmulator();

    CompressorInfo compress(const void *input, void *output, int size);

    CompressorInfo uncompress(const void *input, void *output, int size, int level);

    u32bit getLevelBlockSize(unsigned int level) { 
        GPU_ASSERT(
                if (level >= numLevels)
                    panic("MsaaCompressorEmulator", "getCompressedBlockSize", 
                            "Compressed block size requested for not supported level."); )
        return configs[level].size; }

private:
    bool checkSubblock(const u32bit *data, int start, const MsaaLevelConfig* conf, MsaaRefs* refs);
    int findCompressionLevel(const u32bit *data, int size, MsaaRefs* refsArray, int &refsArraySize);

private:
    // Maximum number of compression levels supported
    static const int maxLevels = 5;

    // One compressor supports numLevels levels of compression, and each
    // level of compression have different configuration parameters.
    static const int numLevels = maxLevels;
    
    static const MsaaLevelConfig configs[maxLevels];

    // number of elements in a block
    int blockSize;
};

}

#endif /*MSAACOMPRESSOREMULATOR_H_*/
