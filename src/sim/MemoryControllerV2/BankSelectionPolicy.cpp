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

#include "BankSelectionPolicy.h"
#include <algorithm>
#include <ctime>
#include <sstream>
#include <iostream>

using namespace std;
using namespace gpu3d;
using namespace gpu3d::memorycontroller;

///////////////////////////////////////////////////////////////////
////////// Factory to create BankSelectionPolicy objects //////////
///////////////////////////////////////////////////////////////////
BankSelectionPolicy* BankSelectionPolicyFactory::create(const std::string& definition, u32bit banks)
{
    stringstream ss(definition);

    BankSelectionPolicy* bsp = new BankSelectionPolicy();

    string policy("UNKNOWN");
    while ( ss >> policy )
    {
        for ( u32bit i = 0; i < policy.length(); ++i ) {
            policy[i] = static_cast<char>(toupper(policy[i]));
        }
        if ( policy == "RANDOM" ) {
            bsp->addPolicy(new RandomComparator(banks));
        }
        else if ( policy == "ROUND_ROBIN" ) {
            bsp->addPolicy(new RoundRobinComparator(banks));
        }
        else if ( policy == "OLDEST_FIRST" ) {
            bsp->addPolicy(new AgeComparator(AgeComparator::OLDEST_FIRST));
        }
        else if ( policy == "YOUNGEST_FIRST" ) {
            bsp->addPolicy(new AgeComparator(AgeComparator::YOUNGEST_FIRST));
        }
        else if ( policy == "MORE_CONSECUTIVE_HITS" ) {
            bsp->addPolicy(new ConsecutiveHitsComparator(ConsecutiveHitsComparator::MORE_CONSECUTIVE_HITS_FIRST));
        }
        else if ( policy == "LESS_CONSECUTIVE_HITS" ) {
            bsp->addPolicy(new ConsecutiveHitsComparator(ConsecutiveHitsComparator::LESS_CONSECUTIVE_HITS_FIRST));
        }
        else if ( policy == "MORE_PENDING_REQUESTS" ) {
            bsp->addPolicy(new QueueSizeComparator(QueueSizeComparator::MORE_PENDING_REQUESTS));
        }
        else if ( policy == "LESS_PENDING_REQUESTS" ) {
            bsp->addPolicy(new QueueSizeComparator(QueueSizeComparator::LESS_PENDING_REQUESTS));
        }
        else if ( policy == "ZERO_PENDING_FIRST" ) {
            bsp->addPolicy(new QueueSizeComparator(QueueSizeComparator::ZERO_PENDING_FIRST));
        }
        else {
            stringstream ssaux;
            ssaux << "Unknown bank selection policy: \"" << policy << "\"";
            panic("BankSelectionPolicyFactory", "create", ssaux.str().c_str());
        }
    }
    return bsp;
}


BankSelectionPolicy::BankSelectionPolicy()
{
    // empty
}

void BankSelectionPolicy::addPolicy(BankInfoComparator* bankComparator)
{
    bankCompareObject.addPolicy(bankComparator);
}

void BankSelectionPolicy::sortBanks(BankInfoArray& banksInfo)
{
    if ( bankCompareObject.ready() ) {
        bankCompareObject.update();
        std::sort(banksInfo.begin(), banksInfo.end(), bankCompareObject);
    }
    else {
        panic("BankSelectionPolicy", "sortBanks", "At least 1 bankCOmparator policy has to be added");
    }
}

void BankSelectionPolicy::debug_printBanks(const BankInfoArray& bia)
{
    for ( u32bit i = 0; i < bia.size(); ++i ) {
        BankSelectionPolicy::BankInfo* b = bia[i];
        cout << "BanksInfo[" << i << "] = (bankID=" << b->bankID << ",age=" << b->age << ",consecHits=" << b->consecHits
            << ",queueSize=" << b->queueSize << ")" << endl;
    }    
}

void BankSelectionPolicy::BankCompareObject::addPolicy(BankInfoComparator* bankComparator)
{
    policies.push_back(bankComparator);
}

bool BankSelectionPolicy::BankCompareObject::ready() const
{
    return !policies.empty();
}

bool BankSelectionPolicy::BankCompareObject::operator()(const BankInfo* a, const BankInfo* b)
{
    s32bit result = 0;
    for ( u32bit i = 0; i < policies.size() && result == 0; ++i )
        result = policies[i]->compare(a,b);
    return result == -1;
}

void BankSelectionPolicy::BankCompareObject::update()
{
    for ( u32bit i = 0; i < policies.size(); ++i )
        policies[i]->update();
}

///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// BankInfoComparator methods ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

RandomComparator::RandomComparator(u32bit banks) : Banks(banks), randomWeights(banks)
{
    // empty
}

void RandomComparator::update()
{
    srand(static_cast<u32bit>(time(0)));
    for ( u32bit i = 0; i < Banks; ++i )
        randomWeights[i] = static_cast<u32bit>(rand());
}

s32bit RandomComparator::compare(const BankSelectionPolicy::BankInfo* a, const BankSelectionPolicy::BankInfo* b)
{
    if ( randomWeights[a->bankID] < randomWeights[b->bankID] )
        return -1;
    else
        return  1;
}

RoundRobinComparator::RoundRobinComparator(u32bit banks) : Banks(banks), nextRR(0)
{
    // empty
}

void RoundRobinComparator::update()
{
    nextRR = (nextRR + 1) % Banks;
}

s32bit RoundRobinComparator::compare(const BankSelectionPolicy::BankInfo* a, const BankSelectionPolicy::BankInfo* b)
{
    if ( a->bankID == b->bankID )
        return 0;

    u32bit turnBankA = (a->bankID + (Banks - nextRR)) % Banks;
    u32bit turnBankB = (b->bankID + (Banks - nextRR)) % Banks;

    if ( turnBankA < turnBankB )
        return -1;
    else // turnBankA > turnBankB
        return 1;
}


AgeComparator::AgeComparator(ComparatorType type) : compType(type)
{
    // empty
}

s32bit AgeComparator::compare(const BankSelectionPolicy::BankInfo* a, const BankSelectionPolicy::BankInfo* b) 
{       
    if ( a->age == b->age ) // Equal age accepted to support "aging"
        return 0;
    if ( a->age < b->age )
        return (compType == OLDEST_FIRST ? -1 : 1);
    else // a->age > b->age
        return (compType == OLDEST_FIRST ? 1 : -1);
}


ConsecutiveHitsComparator::ConsecutiveHitsComparator(ComparatorType type) : compType(type)
{
    // empty
}

s32bit ConsecutiveHitsComparator::compare(const BankSelectionPolicy::BankInfo* a, const BankSelectionPolicy::BankInfo* b)
{       
    if ( a->consecHits == b->consecHits )
        return 0;
    if ( a->consecHits < b->consecHits)
        return (compType == LESS_CONSECUTIVE_HITS_FIRST ? -1 : 1);
    else // a->consecHits > b->consecHits)
        return (compType == LESS_CONSECUTIVE_HITS_FIRST ? 1 : -1);
}


QueueSizeComparator::QueueSizeComparator(ComparatorType type) :compType(type)
{
    // empty
}

s32bit QueueSizeComparator::compare(const BankSelectionPolicy::BankInfo* a, const BankSelectionPolicy::BankInfo* b)
{
    if ( compType == ZERO_PENDING_FIRST ) {
        if ( a->queueSize == 0 && a->queueSize == 0 )
            return 0;
        else if ( a->queueSize == 0 /* && b->queueSize != 0 */ )
            return -1;
        else if ( /* a->queueSize != 0 && */ b->queueSize == 0 )
            return 1;
        else // values different of 0 are equivalent in this comparison mode
            return 0;
    }
    else {
        if ( a->queueSize == b->queueSize )
            return 0;
        if ( a->queueSize < b->queueSize ) 
            return (compType == LESS_PENDING_REQUESTS ? -1 : 1);
        else
             return (compType == LESS_PENDING_REQUESTS ? 1 : -1);
    }
}
