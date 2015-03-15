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

#ifndef SHADERPROGRAMSCHED_H
    #define SHADERPROGRAMSCHED_H

#include <map>
#include <vector>
#include "GPU.h"

class GPUDriver;

class ShaderProgramSched
{
public:

    struct Statistics
    {
        u32bit selectHits; // a program does not require a LOAD command
        u32bit selectMisses; // a program requires a LOAD command
        u32bit totalClears; // shader instruction memory flushes
        u32bit programsInMemory; // current programs in shader instruction memory
        f32bit programsInMemoryMean; // Mean of programs in memory (sampled at each select method call)
        u32bit shaderInstrMemoryUsage; // Currently
        f32bit shaderInstrMemoryUsageMean; // Mean of shader instruction memory used (sampled at each select method call)
    };
    
    static const u32bit InstructionSize = 16; // 16 bytes
    
    ShaderProgramSched(GPUDriver* driver, 
                       u32bit maxInstrSlots);

    Statistics getStatistics() const;
    
    void resetStatistics();

    void clear(); // reset the shader program sched state (equals to remove all md's with remove method)
    void remove(u32bit md); // must be called every time a md is removed from GPU driver
    void select(u32bit md, u32bit nInstr, gpu3d::ShaderTarget target); // add (if required) and select as current
    void loadShaderProgram (gpu3d::ShaderTarget target, u32bit nInstr, u32bit startPC, u32bit md);
    
    u32bit instructionSlots() const;
    
    // defaults to clear the shader instruction memory when is full
    // you can rewrite this method in subclasses to allow a more
    // complex behaviour
    virtual u32bit reclaimRoom(u32bit nInstrSlots, gpu3d::ShaderTarget target);
    
    virtual void dump() const;
    
    void dumpStatistics() const;

protected:

    // Used to release&allocate&check room in the subclasses
    //void release(u32bit md);
    //bool allocate(u32bit md, u32bit startSlot, u32bit nInstrSlots);
    bool isFree(u32bit instructionSlot);
    

private:

    std::vector<bool> free;

    struct ProgInfoStruct
    {
        u32bit startPC; // Start slots
        u32bit nInstr; // # of instruction slots
        gpu3d::ShaderTarget target;    //  Shader target of the program.
    };

    typedef std::map<u32bit,ProgInfoStruct> ProgInfo;
    typedef ProgInfo::iterator ProgInfoIt;
    typedef ProgInfo::const_iterator ProgInfoConstIt;
    // <descriptor, progInfo>
    ProgInfo progInfo;

    GPUDriver* driver;
    
    // internal statistics
    u32bit selectHits;
    u32bit totalSelects;
    u32bit totalClears;
    u32bit programsInMemoryAccum; // Computed each time that select is called    
    u32bit shaderInstrMemoryUsage;
    u32bit shaderInstrMemoryUsageAccum;
    u32bit freeSlots;
    u32bit lastVertexShaderMD;

};

#endif // SHADERPROGRAMSCHED_H
