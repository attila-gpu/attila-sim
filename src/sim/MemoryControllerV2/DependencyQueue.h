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

#ifndef DEPENDENCYQUEUE_H
    #define DEPENDENCYQUEUE_H

#include "GPUTypes.h"
#include <list>
#include <string>

namespace gpu3d
{
namespace memorycontroller
{

class ChannelTransaction;

class DependencyQueue
{
private:

    struct QueueEntry
    {
        ChannelTransaction* ct;
        ChannelTransaction* dependency; // 0 means "No dependency"
        u64bit timestamp;
        QueueEntry(ChannelTransaction* ct, 
                   ChannelTransaction* dependency,
                   u64bit cycleTimestamp ) :
            ct(ct), dependency(dependency), timestamp(cycleTimestamp) {}
    };

    typedef std::list<QueueEntry> Queue;

    Queue q;
    u32bit qSize; // to increase efficiency in implementations with STL list.size() O(n)
    u32bit capacity;
    std::string qName;

public:

    DependencyQueue();


    /**
     * Specific call supported to implement bank selection policies based on consecutive hits to the same page
     */
    u32bit getConsecutiveAccesses(bool writes) const;

    /**
     * Wake up (remove dependency) from all channel transactions in the queue
     * with this dependency 
     *
     * @param dependency all the transactions in the queue that has dependency
     *        as dependency are woken up (dependency removed)
     *
     * @return the transaction woken up (0 if no transaction is woken up)
     */
    void wakeup(const ChannelTransaction* dependency);
    
    /**
     * Returns the channel transaction within the queue that is a dependency 
     * of the request
     *
     * @param request find the transaction in the queue that matches with the request.
     *        That is the transaction that is the dependency of the request
     *
     * @return The dependency transaction (0 if no dependency exists)
     */
    ChannelTransaction* findDependency(const ChannelTransaction* request) const;

    /**
     * Return true is the queue is empty
     */
    bool empty() const;

    /**
     * Returns the current amount of items i
     */
    u32bit size() const;

    /**
     * Add a new channel transaction to the queue and its dependency
     * If the request has no dependency a zero must be provided as dependency
     *
     * @param request the new transaction added to the queue
     * @param dependency the dependency of the request (0 must be supplied if no dep exists)
     */
    void enqueue(ChannelTransaction* request, 
                 ChannelTransaction* dependency,
                 u64bit cycleTimestamp = 0);

    /**
     * Get the timestamp of the next transaction
     */
    u64bit getTimestamp() const;

    /**
     * Check if the next transaction in the queue has a dependency
     *
     * @returns true is the next transaction is ready, false otherwise
     */
    bool ready() const;

    /**
     * Returns a pointer to the next transaction in the queue
     *
     * @returns a pointer to the next transaction in the queue or 0 if the queue is empty
     */
    ChannelTransaction* front() const;

    /**
     * Returns the dependency of the next transaction in the queue
     * It returns 0 if the transaction is ready (has no dependency)
     */
    ChannelTransaction* frontDependency() const;

    /**
     * Remove the next transaction in the queue
     *
     * @warning This function panics if the transaction is not ready
     */
    void pop();

    void setName(const std::string& name);

    const std::string& getName() const;

}; // class DependencyQueue

} // namespace memorycontroller
} // namespace gpu3d

#endif // DEPENDENCYQUEUE_H
