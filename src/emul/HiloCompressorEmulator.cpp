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

#include "HiloCompressorEmulator.h"
#include "BitStreamWriter.h"
#include "BitStreamReader.h"

namespace gpu3d
{

HiloCompressorEmulator::HiloCompressorEmulator(int numLevels, u32bit dataMask, bool reorderData)
    : numLevels(numLevels), dataMask(dataMask), reorderData(reorderData)
{
    static const int lobits[] = {5, 13, 21};
    static const int csize[] = {64, 128, 192};
    
    // Initialize compressed blocks sizes
    for (int i = 0; i < numLevels; i++) {
        configs[i] = HiloLevelConfig(lobits[i]);
        comprSize[i] = csize[i];
    }
}

HiloCompressorEmulator::~HiloCompressorEmulator()
{
}

u32bit HiloCompressorEmulator::reorder(
        u32bit data, const HiloLevelConfig& config)
{
    return reorderData ? 
            (data & config.reHighMask1)
                | (data & config.reHighMask2) << config.reHighShift2
                | (data & config.reHighMask3) << config.reHighShift3
                | (data & config.reHighMask4) << config.reHighShift4
                    
                | (data & config.reLowMask1) >> config.reLowShift1
                | (data & config.reLowMask2) >> config.reLowShift2
                | (data & config.reLowMask3) >> config.reLowShift3
                | (data & config.reLowMask4)
            : data;
}

u32bit HiloCompressorEmulator::unreorder(
        u32bit data, const HiloLevelConfig& config)
{
    return reorderData ? 
            (data & config.reHighMask1)
                | (data & config.unreHighMask2) >> config.reHighShift2
                | (data & config.unreHighMask3) >> config.reHighShift3
                | (data & config.unreHighMask4) >> config.reHighShift4
                
                | (data & config.unreLowMask1) << config.reLowShift1
                | (data & config.unreLowMask2) << config.reLowShift2
                | (data & config.unreLowMask3) << config.reLowShift3
                | (data & config.reLowMask4)
            : data;
}

MinMaxInfo HiloCompressorEmulator::findMinMax(
        const u32bit *data, int size, const HiloLevelConfig& config)
{
    u32bit min = 0xffffffff;
    u32bit max = 0x00000000;
    
    for (int i = 0; i < size; i++) {
        u32bit d = reorder(data[i] & dataMask, config);
        min = d < min ? d : min;
        max = d > max ? d : max;
    }
    
    return MinMaxInfo(min, max);
}

int HiloCompressorEmulator::findCompressionLevel(
        const u32bit *data, int size, MinMaxInfo& mmi, HiloLevelRefs& refs)
{
    int level = 0;

    const HiloLevelConfig* config = &configs[level];
    mmi = findMinMax(data, size, *config);
    refs = HiloLevelRefs(mmi, *config);
    
    int i = 0;
    while (i < size && level < numLevels) {
        u32bit d = reorder(data[i] & dataMask, *config);
        
        while (level < numLevels && !refs.test(d & config->highMask)) {
            level++;
            if (level < numLevels) {
                config = &configs[level];
                mmi = findMinMax(data, size, *config);
                refs = HiloLevelRefs(mmi, *config);
                i = -1;
            }
        }
        i++;
    }
    
    return level;
}

CompressorInfo HiloCompressorEmulator::compress(
        const void *input, void *output, int size)
{
    u32bit* datain = (u32bit*) input;
    u32bit* dataout = (u32bit*) output;
    
    MinMaxInfo mmi;
    HiloLevelRefs refs;
    
    int level = findCompressionLevel(datain, size, mmi, refs);
    bool success = level < numLevels;
    
    if (success) {
        const HiloLevelConfig& config = configs[level];
        
        BitStreamWriter bs(dataout);
        
        //bs.write(level, 2);
        bs.write((mmi.min >> config.lowBits), config.highBits);
        bs.write((mmi.max >> config.lowBits), config.highBits);
        
        for (int i = 0; i < size; i++) {
            u32bit d = reorder(datain[i] & dataMask, config);
            u32bit index = refs.getIndexOfRef(d & config.highMask);
            bs.write(index, 2);
            bs.write(d, config.lowBits);
        }
        
        //volatile int wb = (bs.getWrittenBits() + 7) / 8;
        //cout << wb << " <--> " << (success ? comprSize[level] : (size * sizeof(u32bit))) << endl;
    }
    
    int newSize = success ? comprSize[level] : (size * sizeof(u32bit));
    
    return CompressorInfo(success, level, newSize);
}

CompressorInfo HiloCompressorEmulator::uncompress(
        const void *input, void *output, int size, int level)
{
    u32bit* datain = (u32bit*) input;
    u32bit* dataout = (u32bit*) output;
        
    BitStreamReader bs(datain);
    
    //int level = bs.read(2);
    
    const HiloLevelConfig& config = configs[level];
    
    u32bit min = bs.read(config.highBits) << config.lowBits;
    u32bit max = bs.read(config.highBits) << config.lowBits;
    
    MinMaxInfo mmi(min, max);
    
    HiloLevelRefs refs(mmi, config);
    
    for (int i = 0; i < size; i++) {
        u32bit index = bs.read(2);
        u32bit lowPart = bs.read(config.lowBits);
        u32bit highPart = refs.getRefByIndex(index);
        dataout[i] = unreorder(highPart | lowPart, config);
    }
    
    return CompressorInfo(true, level, size * sizeof(u32bit));
}

}
