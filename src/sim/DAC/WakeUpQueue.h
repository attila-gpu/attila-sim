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

#ifndef WAKEUP_QUEUE_H
    #define WAKEUP_QUEUE_H

#include "GPUTypes.h"
#include "ROPCache.h" // For ROPBlockState definition
#include <vector>
#include <ostream>
#include <set>
#include <list>

namespace gpu3d
{    
/**
  *  Templatized class implementing a wakeup queue. 
  *
  *  It works as follows:
  *  
  *  If the queue is not full, a new entry can be enqueued waiting for a limited set of events (maxEvents). 
  *  When an event is notified to the queue (using the wakeup() operation), 
  *  all the entries waiting for this event are marked. When using the select() operation
  *  the first entry (in enqueue order) having all the events marked is returned. This entry will
  *  be returned until removed from the queue using the remove() operation, and then the next "ready"
  *  entry will be returned.
  *
  *  If an empty event set for an entry is specified, then the entry becomes inmediately selectable.
  *  The event set of an entry can be extended after enqueuing it, up to the maxEvents number, using
  *  the addEventsToEntry() operation. 
  *
  *  IMPORTANT: If an event wakeup action happens before adding the corresponding events to the entry then the entry
  *  won´t be woken up.
  */

template<class Entry, class Event>
struct QueueEntry
{
    Entry* entry;
    std::set<Event> events;  /**<  The events to wait for.  */
    u32bit addedEvents;      /**<  The number of added events up to now.  */
    u32bit totalEvents;      /**<  The total number of events to wait for.  */
    u32bit totalEventsLeft;  /**<  The countdown for the number of events.  */

    QueueEntry(): 
        entry(0), events(std::set<Event>()), addedEvents(0), totalEvents(0), totalEventsLeft(0) {}
        
    QueueEntry(Entry* entry, const std::set<Event>& events, u32bit totalEvents):
        entry(entry), events(events), addedEvents(events.size()), totalEvents(totalEvents), totalEventsLeft(totalEvents) {}
};

template<class Entry, class Event>
class WakeUpQueue
{
private:
    
    std::string name;
    typedef std::list<QueueEntry<Entry, Event> > Queue;
    Queue q;
    
    u32bit maxSize;                 /**<  How many entries the queue can hold as maximum.  */
    u32bit maxEvents;       /**<  How many events each entry can wait as maximum.  */
    

public:

    /**
     * Creates a Wake Up Queue object
     *
     * @note size is set to 0, it must be selected with resize() before add
     * any item. The maximum number of events on-flight is set to 4.
     * This constructor is supplied to allow the creation of arrays of
     * queues with low overhead and unknown size
     */
    WakeUpQueue();

    /**
     * Creates a Wake Up Queue object specifying the initial size
     *
     * @param size maximum number of items that can be hold by the queue.
     * @param size maximum number of events each entry can wait for.
     */
    WakeUpQueue(u32bit size, u32bit maxEvents);
    
    /**
     * Resizes the queue, the previous contents are lost
     *
     */
    void resize(u32bit newSize);

    /**
     * Add a entry to the queue.
     * If the entry has an empty event list to wait for, then the entry becomes inmediately ready/selectable.
     * @note    Throws a panic if entry already exists, the totalEvents is greater than the maximum on-flight
     *          events of the queue or the specified events list size is greater than the totalEvents number.
     *
     * @param entry       The new entry added to the queue.
     * @param events      The list of events to wait for.
     * @param totalEvents The total number of events that will be added to the entry.
     */
    void add(Entry* entry, const std::set<Event>& events, u32bit totalEvents);

    /**
     * Add new events to wait for the specified entry.
     * @note    Throws a panic if the number of already specified entries and the new ones
     *          is greater than the totalEvents number for the entry, or the entry doesn´t
     *            exists.
     *
     * @param entry       The entry to update.
     * @param events      The list of new events to wait for.
     */
    void addEventsToEntry(const Entry* entry, const std::set<Event>& events);
    
    /**
     * Wake up all the queue entries waiting for this event.
     *
     * @param event The current event that will wake up the corresponding
     *              queue entries.
     *
     */
    void wakeup(const Event& event);
                     
    /**
     * Select the oldest next ready/selectable entry of the queue.
     *
     * @return The oldest ready entry in the queue. Return 0 if no ready entries.
     */
    Entry* select() const;
    
    /**
     * Returns if the entry is already present in the wake up queue.
     *
     * @param entry The entry to look up.
     * @return  True if the entry has already been added.
     */
    bool exists(const Entry* entry) const;
    
    /**
     * Removes the entry
     *
     * @param entry Entry to remove.
     * @note        Throws panic if entry not found or non-ready entry.
     *
     */
    void remove(const Entry* entry);
    
    /**
     * Return true is the queue is full
     */
    bool full() const;
    
    /**
     * Return true is the queue is empty
     */
    bool empty() const;

    /**
     * Returns the current amount of items i
     */
    u32bit size() const;

    /**
     * Returns the name of the queue.
     */
    std::string getName() const;
    
    /**
     * Sets Wake Up Queue name.
     */
    void setName(std::string qName);


}; // class WakeUpQueue

/******************************************************************************************
 *                       Templatized Wake up queue class implementation                   *
 ******************************************************************************************/

template<class Entry, class Event>
WakeUpQueue<Entry, Event>::WakeUpQueue()
    : maxSize(0), maxEvents(1), name("Default Queue name")
{
}

template<class Entry, class Event>
WakeUpQueue<Entry, Event>::WakeUpQueue(u32bit size, u32bit maxEvents)
    : maxSize(size), maxEvents(maxEvents), name("Default Queue name")
{
}

template<class Entry, class Event>
void WakeUpQueue<Entry, Event>::resize(u32bit newSize)
{
    maxSize = newSize;
}

template<class Entry, class Event>
std::string WakeUpQueue<Entry, Event>::getName() const
{
    return name;
}

template<class Entry, class Event>
void WakeUpQueue<Entry, Event>::add(Entry* entry, const std::set<Event>& events, u32bit totalEvents)
{
    if (size() == maxSize)
        panic("WakeUpQueue", "add", "Queue is full");

    if (totalEvents > maxEvents)
    {    
        char msg[100];
        sprintf(msg,"The event size list is constraint to max = %d elements", maxEvents);
        panic("WakeUpQueue", "add", msg);
    }
    
    if (events.size() > totalEvents)
        panic("WakeUpQueue", "add", "The totalEvents number is lower than the specified event list size");
        
    if (exists(entry))
        panic("WakeUpQueue", "add", "Entry already exists");
        
    
/*
    cout << "Adding new wakeup queue entry with events: " << endl;
    std::set<Event>::const_iterator iter = events.begin();
    for ( ; iter != events.end(); iter++)
        cout << (*iter);
    cout << endl;
*/        
    q.push_back(QueueEntry<Entry, Event>(entry, events, totalEvents));
}

template<class Entry, class Event>
void WakeUpQueue<Entry, Event>::addEventsToEntry(const Entry* entry, const std::set<Event>& events)
{
    typename Queue::iterator it = q.begin();
    bool found = false;
    
    /*  Search for the corresponding queue entry.  */
    while( it != q.end() && !found)
    {
        if (it->entry == entry)
        {
            if ((it->addedEvents + events.size()) > it->totalEvents)
                panic("WakeUpQueue", "addEventsToEntry", "New added events and previously added sum more than totalEvents specified");
                
            it->addedEvents += events.size();
            
            typename std::set<Event>::const_iterator it2 = events.begin();
            
            for ( ; it2 != events.end(); it2++)
                it->events.insert(*it2);
            
            found = true;
        }
        it++;
    }
    
    if (!found)
        panic("WakeUpQueue","addEventsToEntry","The entry doesn´t exists");
}

template<class Entry, class Event>
Entry* WakeUpQueue<Entry, Event>::select() const
{
    typename Queue::const_iterator it = q.begin();
    
    /* Select/return as ready entries as indicated. */
    while( it != q.end() )
    {
        if (it->totalEventsLeft == 0)
            return it->entry;
            
        it++;
    }

    return 0;
}


template<class Entry, class Event>
bool WakeUpQueue<Entry, Event>::exists(const Entry* entry) const
{
    typename Queue::const_iterator it = q.begin();
    bool found = false;
    
    while(it != q.end() && !found)
    {
        if (it->entry == entry) found = true;
        it++;
    }
    
    return found;
}

template<class Entry, class Event> 
void WakeUpQueue<Entry, Event>::wakeup(const Event& event)
{
    typename Queue::iterator it = q.begin();
    
    bool listenerFound = false;
    
    /* Iterate over all the que entries looking for the events. */
    for ( ; it != q.end(); it++ )
    {
        typename std::set<Event>::iterator it2 = it->events.find(event);
        
        if (it2 != it->events.end())
        {
            listenerFound = true;
            it->events.erase(it2);
            it->totalEventsLeft--;
        }
    }
    
    if (!listenerFound)
    {
        panic("WakeUpQueue", "wakeup", "Any entry waits for the event.");
    }
}

template<class Entry, class Event> 
void WakeUpQueue<Entry, Event>::remove(const Entry* entry)
{
    u32bit position = 0;
    
    if (empty())
        panic("WakeUpQueue", "remove", "Queue is empty");

    typename Queue::iterator it = q.begin();
    typename Queue::iterator next;
    
    bool removed = false;
    
    while(it != q.end() && !removed)
    {
        if (position >= maxSize)
            panic("WakeUpQueue", "remove", "Entry not found");
        
        if (it->entry == entry)
        {
            if (it->totalEventsLeft)
                panic("WakeUpQueue", "remove", "Entry isn´t ready/selectable");
                
            next = it;
            next++;
            q.erase(it);
            it = next;
            removed = true;
        }
        else
        {
            it++;
        }
        
        position++;
    }
    
}


template<class Entry, class Event>
bool WakeUpQueue<Entry, Event>::full() const
{
    return (size() == maxSize);
}


template<class Entry, class Event>
bool WakeUpQueue<Entry, Event>::empty() const
{
    return q.empty();
}

template<class Entry, class Event>
u32bit WakeUpQueue<Entry, Event>::size() const
{
    return q.size();
}

template<class Entry, class Event>
void WakeUpQueue<Entry, Event>::setName(std::string qName)
{
    name = qName;
}


} // namespace gpu3d

#endif // WAKEUP_QUEUE_H
