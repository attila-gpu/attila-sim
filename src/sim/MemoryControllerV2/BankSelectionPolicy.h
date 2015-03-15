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

#ifndef BANKSELECTIONPOLICY_H
    #define BANKSELECTIONPOLICY_H

#include "GPUTypes.h"
#include <vector>
#include <string>

namespace gpu3d {
namespace memorycontroller {

class BankSelectionPolicy;

/**
 * Factory intended to create Bank SelectionPolicy objects based on a textual description
 *
 * @code
 *     BankSelectionPolicy* bsp = bspFactory->create("MORE_CONSECUTIVE_HITS OLDEST_FIRST RANDOM");
 * @endcode
 *
 * This call would create a bank selection policy that utilizes firstly the More Consetuve Hits policy
 * to select a request and in case of a tie, would utilize the Oldest First criteria and in a new
 * tie would utilize a random policy among the canditates filtered by the two previous policies
 */
class BankSelectionPolicyFactory
{
public:

    /**
     * Bank selection policies supported:
     *
     * "RANDOM" : selects a random bank
     * "ROUND_ROBIN" : sorts banks in a round robin fashion each time a BankSelectionPolicy::sortBanks() is called
     * "OLDEST_FIRST" : sorts banks in an oldest first fashion (based on age)
     * "YOUNGEST_FIRST" : sorts banks in an youngest first fashion (based on age)
     * "MORE_CONSECUTIVE_HITS" : sorts banks using the consecutive requests expected to each bank (banks with more first)
     * "LESS_CONSECUTIVE_HITS" : sorts banks using the consecutive requests expected to each bank (banks with less first)
     * "MORE_PENDING_REQUESTS" : sorts banks based on the number of pending requests ( aka queue size), more requests first
     * "LESS_PENDING_REQUESTS" : sorts banks based on the number of pending requests ( aka queue size), less requests first
     * "ZERO_PENDING_FIRST" : classifies banks between banks without pending reqs and banks with pending reqs (first banks without pending requests)
     */
    static BankSelectionPolicy* create(const std::string& definition, u32bit banks);

}; // class BankSelectionPolicyFactory



/**
 * Class responsible of manage a complex bank selection policy combining basic bank selection policies
 */
class BankSelectionPolicy
{
public:

    /**
     * This struct can be augmented with more info required by future bank selection policies 
     */
    struct BankInfo
    {
        u32bit bankID;
        u64bit age;
        u32bit queueSize;
        u32bit consecHits;
    };

    typedef std::vector<BankInfo*> BankInfoArray;

    /**
     * Implement your policy extending from that object
     */
    class BankInfoComparator 
    {
    public:

        /**
         * This method will be called every time just before sorting the banks
         * It is helful for setting up internal state required by BankInfoComparators
         */
        virtual void update() { /* empty by default */ }

        /**
         * Method automatically called that decides what bank has more priority based on the comparison being implemented
         *
         * Expected values: 
         *    -1 if 'a' has more priority than 'b
         *     0 if both have the same priority
         *     1 if 'b' has more priority than 'a'
         */
        virtual s32bit compare(const BankInfo* a, const BankInfo* b) = 0;
        
    };

    /**
     * Add bank compare policies. Each new added policy is used as a tie-breaker in case the
     * previous policy cannot determine which bank has to be selected
     */
    void addPolicy(BankInfoComparator* bic);

    void sortBanks(BankInfoArray& bia);

    static void debug_printBanks(const BankInfoArray& bia);

    BankSelectionPolicy();

private:

    BankSelectionPolicy(const BankSelectionPolicy&);
    BankSelectionPolicy& operator=(const BankSelectionPolicy&);

    class BankCompareObject
    {
    private:
        std::vector<BankInfoComparator*> policies;
    public:
        void update();
        void addPolicy(BankInfoComparator* bankComparator);
        bool operator()(const BankInfo* a, const BankInfo* b);
        bool ready() const;
    };
    BankCompareObject bankCompareObject;

}; // class BankSelectionPolicy


///////////////////////////////////////////////////////////////////////////
////////////////// BankInfoComparator definitions /////////////////////////
///////////////////////////////////////////////////////////////////////////

class RandomComparator : public BankSelectionPolicy::BankInfoComparator
{
public:

    RandomComparator(u32bit banks);
    s32bit compare(const BankSelectionPolicy::BankInfo* a, const BankSelectionPolicy::BankInfo* b);
    void update();

private:

    RandomComparator(const RandomComparator&);
    RandomComparator& operator=(const RandomComparator&);

    const u32bit Banks;
    std::vector<u32bit> randomWeights;

}; // class RandomComparator

class RoundRobinComparator : public BankSelectionPolicy::BankInfoComparator
{
public:

    RoundRobinComparator(u32bit banks);

    s32bit compare(const BankSelectionPolicy::BankInfo* a, const BankSelectionPolicy::BankInfo* b);
    void update();

private:

    RoundRobinComparator(const RoundRobinComparator&);
    RoundRobinComparator& operator=(const RoundRobinComparator&);

    const u32bit Banks;
    u32bit nextRR;
}; // class RoundRobinComparator


class AgeComparator : public BankSelectionPolicy::BankInfoComparator
{
public:

    enum ComparatorType
    {
        OLDEST_FIRST,
        YOUNGEST_FIRST
    };

    AgeComparator(ComparatorType type);
    s32bit compare(const BankSelectionPolicy::BankInfo* a, const BankSelectionPolicy::BankInfo* b);

private:

    ComparatorType compType;
}; // class AgeComparator

class ConsecutiveHitsComparator : public BankSelectionPolicy::BankInfoComparator
{
public:

    enum ComparatorType
    {
        LESS_CONSECUTIVE_HITS_FIRST,
        MORE_CONSECUTIVE_HITS_FIRST
    };

    ConsecutiveHitsComparator(ComparatorType type);
    s32bit compare(const BankSelectionPolicy::BankInfo* a, const BankSelectionPolicy::BankInfo* b);

private:

    ComparatorType compType;
    
}; // class ConsecutiveHitsComparator


class QueueSizeComparator : public BankSelectionPolicy::BankInfoComparator
{
public:

    enum ComparatorType
    {
        LESS_PENDING_REQUESTS,
        MORE_PENDING_REQUESTS,
        ZERO_PENDING_FIRST,
    };

    QueueSizeComparator(ComparatorType type);

    s32bit compare(const BankSelectionPolicy::BankInfo* a, const BankSelectionPolicy::BankInfo* b);

private:

    ComparatorType compType;
};

} // namespace memorycontroller
} // namespace gpu3d


#endif // BANKSELECTIONPOLICY_H
