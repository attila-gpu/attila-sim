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

#include "ShaderStats.h"
#include "support.h"

using namespace std;

InstructionCount::InstructionCount(const string& name, unsigned int progType,
                                   ShProgramManager& spm) :
    GLIStat(name), spm(spm), frameCount(0), numberOfBatches(0), enabled(false)
{
    if ( progType == GL_VERTEX_PROGRAM_ARB )
        isVsh = true;
    else if ( progType == GL_FRAGMENT_PROGRAM_ARB )
        isVsh = false;
    else
        panic("InstructionCount", "InstructionCount", "Unknown program type");
}


int InstructionCount::getBatchCount()
{
    ShProgramInfo* spi = 0;
    int batchCount;
    if ( isVsh )
        spi = spm.getCurrent(GL_VERTEX_PROGRAM_ARB);
    else
        spi = spm.getCurrent(GL_FRAGMENT_PROGRAM_ARB);

    if ( spi && enabled)
    {
        batchCount = spi->countInstructions();
        frameCount += batchCount;    
    }
    else
        batchCount = 0;

    numberOfBatches++;

    return batchCount;
}

int InstructionCount::getFrameCount()
{
    char msg[256];
    
    if ( numberOfBatches == 0 )
        return 0;

    int temp = frameCount / numberOfBatches;
    numberOfBatches = 0;
    frameCount = 0;
    return temp;
}

void InstructionCount::setEnabled(bool enable)
{
    enabled = true;
}

bool InstructionCount::isEnabled() const
{
    return enabled;
}


TextureLoadsCount::TextureLoadsCount(const string& name, ShProgramManager& spm) :
    GLIStat(name), spm(spm), frameCount(0), numberOfBatches(0), enabled(false)
{}


int TextureLoadsCount::getBatchCount()
{
    int batchCount;
    ShProgramInfo* spi = spm.getCurrent(GL_FRAGMENT_PROGRAM_ARB);
    if ( spi && enabled )
    {
        batchCount = spi->countTextureLoads();
        frameCount += batchCount;
        
    }
    else
        batchCount = 0;
    
    numberOfBatches++;

    return batchCount;
}

int TextureLoadsCount::getFrameCount()
{
    if ( numberOfBatches == 0 )
        return 0;

    int temp = frameCount / numberOfBatches;
    frameCount = 0;
    numberOfBatches = 0;
    return temp;
}

void TextureLoadsCount::setEnabled(bool enable)
{
    enabled = true;
}

bool TextureLoadsCount::isEnabled() const
{
    return enabled;
}
