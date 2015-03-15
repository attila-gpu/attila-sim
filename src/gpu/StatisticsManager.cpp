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

#include "StatisticsManager.h"
#include "support.h"
#include <algorithm>

using namespace gpu3d;
using namespace gpu3d::GPUStatistics;
using namespace std;

// StatisticsManager* StatisticsManager::sm = 0;

StatisticsManager::StatisticsManager():
startCycle(0), nCycles(1000), nextDump(999), lastCycle(-1), autoReset(true),
osCycle(NULL), osFrame(NULL), osBatch(NULL), cyclesFlagNamesDumped(false)
{
}

StatisticsManager& StatisticsManager::instance()
{
    static StatisticsManager* sm = new StatisticsManager();
    return *sm;
}

Statistic* StatisticsManager::find(std::string name)
{
    map<string,Statistic*>::iterator it = stats.find(name);
    if ( it == stats.end() )
        return 0;
    return it->second;
}

void StatisticsManager::clock(u64bit cycle)
{
    // static bool flag = false;
    lastCycle = cycle;

    if ( cycle >= startCycle )
        Statistic::enable();
    else
        Statistic::disable();

    if ( cycle >= nextDump )
    {
        if ( !cyclesFlagNamesDumped )
        {
            cyclesFlagNamesDumped = true;
            
            if (osCycle != NULL)
                dumpNames(*osCycle);
        }

        if (osCycle != NULL)
            dumpValues(*osCycle);

        if ( autoReset )
        {
            startCycle = cycle+1;
            reset(FREQ_CYCLES);
        }
        nextDump = cycle + nCycles;
    }
}

void StatisticsManager::frame(u32bit frame)
{
    static bool namesOut = false;

    //  Check if output stream for per frame statistics is defined.
    if (osFrame != NULL)
    {
        if (namesOut == false)
        {
            namesOut = true;
            dumpNames("Frame", *osFrame);
        }

        dumpValues(frame, FREQ_FRAME, *osFrame);

        reset(FREQ_FRAME);
    }
}

void StatisticsManager::batch()
{
    static bool namesOut = false;
    static u32bit batch = 0;

    //  Check if the output stream for per batch statistics is defined
    if (osBatch != NULL)
    {
        if (namesOut == false)
        {
            namesOut = true;
            dumpNames("Batch", *osBatch);
        }

        dumpValues(batch, FREQ_BATCH, *osBatch);

        batch++;

        reset(FREQ_BATCH);
    }
}

Statistic* StatisticsManager::operator[](std::string statName)
{
    return find(statName);
}


void StatisticsManager::setDumpScheduling(u64bit startCycle, u64bit nCycles, bool autoReset)
{
    this->startCycle = startCycle;
    this->nextDump= startCycle+nCycles-1;
    this->nCycles = nCycles; /* dump every 'nCycles' cycles */
    this->autoReset = autoReset;
}

void StatisticsManager::setOutputStream(ostream& os)
{
    osCycle = &os;
}

void StatisticsManager::setPerFrameStream(ostream& os)
{
    osFrame = &os;
}

void StatisticsManager::setPerBatchStream(ostream& os)
{
    osBatch = &os;
}

void StatisticsManager::reset(u32bit freq)
{
    map<string,Statistic*>::iterator it = stats.begin();
    for ( ; it != stats.end(); it++ )
        (it->second)->clear(freq);
}

void StatisticsManager::dumpValues(ostream& os)
{
    map<string,Statistic*>::iterator it = stats.begin();

    os << startCycle << ".." << lastCycle;
    for ( ; it != stats.end(); it++ )
    {
        (*(it->second)).setCurrentFreq(FREQ_CYCLES);
        os << ";" << *(it->second);
    }
    os << endl;
}

void StatisticsManager::dumpValues(u32bit n, u32bit freq, ostream& os)
{
    map<string,Statistic*>::iterator it = stats.begin();

    os << n;
    for ( ; it != stats.end(); it++ )
    {
        (*(it->second)).setCurrentFreq(freq);
        os << ";" << *(it->second);
    }

    os << endl;
}


void StatisticsManager::dumpNames(ostream& os)
{
    map<string,Statistic*>::iterator it = stats.begin();

    os << "Cycles";
    for ( ; it != stats.end(); it++ )
        os << ";" << it->first;
    os << endl;
}

void StatisticsManager::dumpNames(char *str, ostream& os)
{
    map<string,Statistic*>::iterator it = stats.begin();

    os << str;
    for ( ; it != stats.end(); it++ )
        os << ";" << it->first;
    os << endl;
}

void StatisticsManager::dump(ostream& os)
{
    map<string,Statistic*>::iterator it = stats.begin();
    for ( ; it != stats.end(); it++ )
    {
        os << it->second->getName() << " = " << *(it->second) << endl;
    }
}


void StatisticsManager::dump(string boxName, ostream& os)
{
    map<string,Statistic*>::iterator it = stats.begin();
    for ( ; it != stats.end(); it++ )
    {
        if ( it->second->getOwner() == boxName )
            os << it->second->getName() << " = " << *(it->second) << endl;
    }

}

void StatisticsManager::finish()
{
    u64bit prevDump = nextDump + 1 - nCycles;
    if ( lastCycle > prevDump ) {
        if ( osCycle ) {
            if ( !cyclesFlagNamesDumped )
                dumpNames(*osCycle);
            dumpValues(*osCycle);
        }    
    }
}
