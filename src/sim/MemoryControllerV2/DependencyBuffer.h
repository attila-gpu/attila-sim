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

#ifndef DEPENDENCYBUFFER_H
    #define DEPENDENCYBUFFER_H

#include "GPUTypes.h"
#include "ChannelTransaction.h"
#include <list>
#include <vector>
#include <set>

namespace gpu3d
{
namespace memorycontroller
{

class DependencyBuffer
{

public:

    enum BufferPolicy
    {
        BP_Fifo, // For debug purposes (Dependency buffer behaves like a FIFO)
        BP_OldestFirst,
    };

    typedef u32bit BufferEntry;

    static const BufferEntry NoEntry = 0xFFFFFFFF;

    DependencyBuffer();

    void setBufferPolicy( BufferPolicy bp );

    void add(ChannelTransaction* ct, u64bit timestamp);

    // Extracts the transaction and wakes up all dependent transactions
    ChannelTransaction* extract( BufferEntry entry );

    const ChannelTransaction* getTransaction( BufferEntry entry ) const;

    // Check if exists a pending transaction targeting a given page
    u32bit countHits(u32bit page, bool findWrites) const;    

    u64bit getTimestamp( BufferEntry entry );
    u32bit getAccesses(BufferEntry entry) const;

    bool empty() const;

    u32bit size() const;

    // Gets a candidate preferring to hit a specific page
    BufferEntry getCandidate(bool findWrite, u32bit preferredRow ) const;
    // Gets a candidate without try to match a specific page
    BufferEntry getCandidate(bool findWrite) const;

    bool ready(BufferEntry entry) const;

    void setName(const std::string& name);

    const std::string& getName() const;

    void dump() const;


private:

    struct _BufferEntry
    {
        ChannelTransaction* ct;
        const ChannelTransaction* dependency;
        u64bit timestamp;

        _BufferEntry(ChannelTransaction* ct, const ChannelTransaction* dependency, u64bit timestamp) :
            ct(ct), dependency(dependency), timestamp(timestamp) {}
    };

    typedef std::list<_BufferEntry> _Buffer;
    typedef _Buffer::iterator _BufferIter;
    typedef _Buffer::const_iterator _BufferConstIter;

    _BufferConstIter _findEntry( BufferEntry entry, const char* callerMethodName) const;

    _Buffer _buffer;
    BufferPolicy _bufferPolicy;
    u32bit _entries;
    std::string _name;
    mutable BufferEntry _lastAccessed;
    mutable _BufferConstIter _lastAccessedIter;

    const ChannelTransaction* _findDependency(const ChannelTransaction* ct) const;
    void _wakeup(const ChannelTransaction*);

};

} // namespace memorycontroller
} // namespace gpu3d

#endif // DEPENDENCYBUFFER_H
