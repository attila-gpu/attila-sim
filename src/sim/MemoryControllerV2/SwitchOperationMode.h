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

#ifndef SWITCHOPERATIONMODE_H
    #define SWITCHOPERATIONMODE_H

#include "GPUTypes.h"
#include "ChannelScheduler.h"

namespace gpu3d 
{
namespace memorycontroller 
{

class SwitchOperationModeBase
{
public:

    virtual bool reading() const = 0;
    
    bool writing() const { return !reading(); }

    virtual void update(bool reads, bool writes, bool readIsHit, bool writeIsHit) = 0;

    virtual u32bit moreConsecutiveOpsAllowed() const = 0;

    virtual u32bit MaxConsecutiveReads() const = 0;
    
    virtual u32bit MaxConsecutiveWrites() const = 0;
};


class SwitchOperationModeSelector
{
public:

    static SwitchOperationModeBase* create(const ChannelScheduler::CommonConfig& config);
};


class SwitchModeTwoCounters : public SwitchOperationModeBase
{

private:

    friend class SwitchOperationModeSelector;

    SwitchModeTwoCounters(u32bit maxConsecutiveReads, u32bit maxConsecutiveWrites);

public:

    // Called before the selection between a read and a write.
    // After calling this method call reading() to decide if a read or write should be selected.
    void update(bool reads, bool writes, bool readIsHit, bool writeIsHit);

    bool reading() const;
    
    u32bit moreConsecutiveOpsAllowed() const;

    u32bit MaxConsecutiveReads() const;
    
    u32bit MaxConsecutiveWrites() const;

private:

    const u32bit _MaxConsecutiveReads;
    const u32bit _MaxConsecutiveWrites;
    u32bit _consecutiveOps;
    bool _reading;

};

class SwitchModeLoadOverStores : public SwitchOperationModeBase
{
private:

    friend class SwitchOperationModeSelector;

    SwitchModeLoadOverStores();

public:

    // Called before the selection between a read and a write.
    // After calling this method call reading() to decide if a read or write should be selected.
    void update(bool reads, bool writes, bool readIsHit, bool writeIsHit);

    bool reading() const;
    
    u32bit moreConsecutiveOpsAllowed() const;

    u32bit MaxConsecutiveReads() const;
    
    u32bit MaxConsecutiveWrites() const;

private:

    bool _reading;


};


class SwitchModeCounters
{
public:

    SwitchModeCounters(u32bit maxConsecutiveReads, u32bit maxConsecutiveWrites);

    bool reading();
    bool writing();
    void switchMode();
    void newOp();
    u32bit moreConsecutiveOpsAllowed();

    u32bit MaxConsecutiveReads() const;
    u32bit MaxConsecutiveWrites() const;

private:

    const u32bit _MaxConsecutiveReads;
    const u32bit _MaxConsecutiveWrites;
    u32bit _consecutiveOps;
    bool _writing;

};


} // namespace memorycontroller
} // namespace gpu3d



#endif // SWITCHOPERATIONMODE_H
