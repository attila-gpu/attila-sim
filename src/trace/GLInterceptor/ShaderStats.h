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

#ifndef SHADERSTATS_H
    #define SHADERSTATS_H

#include "GLIStat.h"
#include "ShProgramInfo.h"

class InstructionCount : public GLIStat
{
private:

    /**
     * only taking into account batches with fp or vp enabled
     */
    int numberOfBatches; // maintains the number of batches up to now
    
    int frameCount; // batch count not needed
    ShProgramManager& spm;
    bool isVsh; // true -> vertex program, false -> fragment program
    bool enabled;

public:

    InstructionCount(const std::string& name, unsigned int progType, ShProgramManager& spm);

    virtual int getBatchCount();

    virtual int getFrameCount();

    /**
     * Sets if this program is this kind of program is enabled or disabled
     */
    void setEnabled(bool enable);

    /**
     * Check if this kind of program is disabled or enabled
     */
    bool isEnabled() const;
};

class TextureLoadsCount : public GLIStat
{
private:

    int numberOfBatches; // maintains the number of batches up to now
    int frameCount; // batch count not needed
    ShProgramManager& spm;

    bool enabled;

public:

    TextureLoadsCount(const std::string& name, ShProgramManager& spm);

    virtual int getBatchCount();

    virtual int getFrameCount();

    /**
     * Sets if this program is this kind of program is enabled or disabled
     */
    void setEnabled(bool enable);

    /**
     * Check if this kind of program is disabled or enabled
     */
    bool isEnabled() const;

};

#endif
