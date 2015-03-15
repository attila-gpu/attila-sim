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

#ifndef MEMORYREQUEST_H
    #define MEMORYREQUEST_H

#include "GPUTypes.h"
#include "MemoryTransaction.h"
#include <vector>


namespace gpu3d
{
namespace memorycontroller
{

class ChannelTransaction; // forward declaration

class MemoryRequest
{
public:

    MemoryRequest(MemoryTransaction* mt = 0, u64bit arrivalTime = 0);

    void setArrivalTime(u64bit arrivalTime);
    u64bit getArrivalTime() const;

    u32bit getID() const;
    bool isRead() const; // false if it is a write
    bool isMasked() const;
    const u32bit* getMask() const;
    MemoryTransaction* getTransaction() const;

    void setState(MemReqState state);
    MemReqState getState() const;
    void setTransaction(MemoryTransaction* mt);
    
    void setOccupied(bool inUse);
    bool isOccupied() const;

    void setCounter(u32bit initialValue);
    void decCounter();
    u32bit getCounter() const;

    // to compute average time to serve a transaction
    void setReceivedCycle(u64bit cycle);
    u64bit getReceivedCycle() const;

    void dump() const;

private:

    MemoryTransaction* memTrans;
    u32bit counter;
    bool occupied;
    MemReqState state;
    u64bit arrivalTime;

};

} // namespace memorycontroller
} // namespace gpu3d


#endif // MEMORYREQUEST_H
