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
 * $RCSfile: GPUSignal.cpp,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2007-11-18 20:13:54 $
 *
 * Signal class implementation file.
 *
 */

#include "GPUSignal.h"
#include "QuadFloat.h"
#include <iostream>
#include <sstream>
#include <cmath>

using namespace gpu3d;
using namespace std;

Signal::Signal( const char* signalName, u32bit bandwidth, u32bit latency ) :
maxLatency(latency), bandwidth(bandwidth), //capacity(maxLatency+1),
nWrites(0), readsDone(0), lastRead(0), lastWrite(0), lastCycle(0), in(0),
nextRead(0), nextWrite(maxLatency), pendentReads(0)
{
    // Data structure creation and initialization
    name = new char[strlen(signalName)+1];
    strcpy( name, signalName );
    create();
}


Signal::~Signal()
{
    // free dynamic memory
    delete[] name;
    if ( isSignalDefined() )
        destroy();
}


// pre bigbang set up !
bool Signal::setData( DynamicObject* initialData[], u64bit firstCycleForReadOrWrite )
{
    if ( bandwidth <= 0 || maxLatency <= 0 )
        panic("Signal", "setData", "Error. Signal is not well defined yet. It Cannot be initializated");

    u32bit i = 0; // for indexing initialData array
    // note: Using global in
    in = static_cast<u32bit>(GPU_MOD( firstCycleForReadOrWrite, capacity )); // mapping of 'i' in signal contents
    for ( ; i < maxLatency; i++ ) {
        for ( nReads[in] = 0; nReads[in] < bandwidth; nReads[in]++ ) {
            u32bit aux = i*bandwidth + nReads[in];
            data[in][nReads[in]] = initialData[aux];
            pendentReads++;
        }
        in = GPU_MOD( in + 1, capacity ); // next in
    }
    readsDone = 0;
    lastCycle = lastRead = lastWrite = firstCycleForReadOrWrite;

    return true;
}

void Signal::dump() const
{

    if ( !isSignalDefined() ) {
        cout << "Signal undefined" << endl;
        cout << "Bandwidth: " << bandwidth << endl;
        cout << "Max. Latency: " << maxLatency << endl;
        return ;
    }

    u32bit i, j;

    for ( i = 0; i < bandwidth; i++ ) {
        for ( j = 0; j < maxLatency + 1; j++ ) {
            if ( data[j][i] == 0 )
                cout << "0 ";
            else
                cout << data[j][i] << " "; // prints address like an integer
        }
        cout << endl;
    }
    for ( i = 0; i < (maxLatency + 1)*2-1; i++ )
        cout << "-";
    cout << endl;
    for ( i = 0; i < maxLatency + 1; i++ )
        cout << nReads[i] << " ";
    cout << endl;
    cout << "Successfully writes in last cycle(" << lastWrite << ")  : " << nWrites << endl;
    cout << "Reads pendents in last cycle(" << lastRead << "): "
        << nReads[in] - readsDone << endl;
    cout << "Successfully reads done in last cycle(" << lastRead << "): " << readsDone << endl;
}

/*  Update the signal clock.  */
void Signal::clock(u64bit cycle)
{
    /*  Calculate the number cycles that has passed since the last clock.  */
    u32bit passedCycles = u32bit(cycle - lastCycle);

    /*  Test if there are pendent reads.  */
    if (pendentReads == 0)
    {
        /*  Move the signal read point by the number of passed cycles .  */
        nextRead = (nextRead + passedCycles) & capacityMask;

        /*  Move the signal write point by the number of passed cycles .  */
        nextWrite = (nextWrite + passedCycles) & capacityMask;

        /*  Update cycle.  */
        lastCycle = cycle;

        /*  No reads done in this cycle.  */
        readsDone = 0;
    }
    else if (pendentReads > 0)
    {
        /*  Test if only a cycle has passed and there are no more reads for the previous cycle.  */
        if ((cycle == (lastCycle + 1)) && (nReads[nextRead] == 0))
        {
            /*  Move the signal read point by cycle.  */
            nextRead = (nextRead + 1) & capacityMask;

            /*  Move the signal write point by the number of passed cycles .  */
            nextWrite = (nextWrite + 1) & capacityMask;

            /*  Update cycle.  */
            lastCycle = cycle;

            /*  No reads done in this cycle.  */
            readsDone = 0;
        }
        /*  Test if a period longer than the maximum latency has not passed.  */
        else if (cycle <= (lastCycle + maxLatency))
        {
            /*  No missed reads found yet.  */
            u32bit missedReads = 0;

            /*  Test if there are pending reads in the cycles that have passed since the last clock update.  */
            for(u32bit i = 0; i < passedCycles; i++)
            {
                /*  Calculate the number of missed reads.  */
                missedReads += nReads[nextRead];

                /*  Move the signal read point by cycle.  */
                nextRead = (nextRead + 1) & capacityMask;
            }

            /*  Check if there are missed reads in the cycles that have passed.  */
            if (missedReads > 0)
            {
                char buffer[1024];
                sprintf(buffer, "Lost data in signal \"%s\" cycle %lld.", name, cycle);
                panic("Signal", "clock", buffer);
            }

            /*  Move the signal write point by the number of passed cycles .  */
            nextWrite = (nextWrite + passedCycles) & capacityMask;

            /*  Update cycle.  */
            lastCycle = cycle;

            /*  No reads done in this cycle.  */
            readsDone = 0;
        }
        else
        {
            char buffer[1024];
            sprintf(buffer, "Lost data in signal \"%s\" cycle %lld.", name, cycle);
            panic("Signal", "clock", buffer);
        }
    }

    // if (cycle < lastCycle)
    //{
    //    char buffer[200];
    //    sprintf(buffer, "Going back to the past at signal \"%s\" cycle %lld.", name, cycle);
    //    panic("Signal", "clock", buffer);
    //}
}

void Signal::clockFast(u64bit cycle)
{
    /*  Check for missed reads.  */

    /*  Move the signal read point by the number of passed cycles .  */
    nextRead = (nextRead + u32bit(cycle - lastCycle)) & capacityMask;

    /*  Move the signal write point by the number of passed cycles .  */
    nextWrite = (nextWrite + u32bit(cycle - lastCycle)) & capacityMask;

    /*  Update cycle.  */
    lastCycle = cycle;

    /*  No reads done in this cycle.  */
    readsDone = 0;

}

std::string Signal::strFormatObject(const DynamicObject* dynObj)
{
    return string("CLASS=\"") + dynObj->getClass() + "\"  STRING=\"" + dynObj->toString() + "\"";
}

bool Signal::writeGenFast(u64bit cycle, DynamicObject* dataW)
{
    /*  Update the signal clock.  */
    if (cycle > lastCycle)
        clock(cycle);

//printf("writeGenFast >> signal %s >> cycle %lld nextWrite %d nReads %d\n", name, cycle,
//nextWrite, nReads[nextWrite]);

    /*  Test allocated slots (bandwidth usage) at the insertion point.  */
    if (nReads[nextWrite] == bandwidth)
    {
        std::stringstream ss;
        ss << "Error.  Max. BW exceeded (read conflict).  Signal \"" << name << "\" cycle " << cycle << ". "
            "\n  New written object -> " << strFormatObject(dataW) <<
            "\n  Collides with: \n";
        for ( u32bit i = 0; i < bandwidth; ++i ) {
            ss << "i=" << i << " -> " << strFormatObject(data[nextWrite][i]) << "\n";
        }
        panic("Signal", "writeGen", ss.str().c_str());
     }

    /*  Insert the incoming object in the insertion point.  */
    data[nextWrite][nReads[nextWrite]] = dataW;

    /*  Update allocated slots at the insertion point.  */
    nReads[nextWrite]++;

    /*  Update pendent reads.  */
    pendentReads++;

    return true;
}


bool Signal::writeGenFast(u64bit cycle, DynamicObject* dataW, u32bit latency)
{
    /*  Update the signal clock.  */
    if (cycle > lastCycle)
        clock(cycle);

    /*  Test the insertion latency against the maximum allowed latency.  */
    if (latency > maxLatency)
    {
        char buffer[200];

        sprintf(buffer, "Error.  Inconsistent latency value. Signal %s, latency %d."
                        ".DynamicObject info='%s'\n", name, latency, (char *)dataW->getInfo());
        panic("Signal", "writeGen", buffer);
    }

    nextWrite = (nextRead + latency) & capacityMask;

//printf("writeGenFast >> signal %s >> cycle %lld latency %d nextWrite %d nReads %d\n", name, cycle,
//latency, nextWrite, nReads[nextWrite]);

    /*  Test allocated slots (bandwidth usage) at the insertion point.  */
    if (nReads[nextWrite] == bandwidth)
    {
        char buffer[200];

        sprintf(buffer, "Error.  Max. BW exceeded (read conflict).  Signal \"%s\" cycle %d. "
                        "DynamicObject info='%s'\n", name, cycle, (char *)dataW->getInfo());

        panic("Signal", "writeGen", buffer);
     }

    /*  Insert the incoming object in the insertion point.  */
    data[nextWrite][nReads[nextWrite]] = dataW;

    /*  Update allocated slots at the insertion point.  */
    nReads[nextWrite]++;

    /*  Update pendent reads.  */
    pendentReads++;

    return true;
}

bool Signal::readGenFast(u64bit cycle, DynamicObject*& dataR)
{
    /*  Calculates if a cycle has passed since the last access to the signal.  */
    bool cycleChanged = (cycle > lastCycle);

    /*  Check if there are pendent reads.  */
    if ((pendentReads == 0) || (!cycleChanged && (nReads[nextRead] == 0)))
        return false;

    /*  Update the signal clock.  */
    if (cycleChanged)
        clock(cycle);

//printf("readGenFast >> signal %s >> cycle %lld nextRead %d nReads %d\n", name,
//cycle, nextRead, nReads[nextRead]);

    /*  Check if there is data in the signal.  */
    if (nReads[nextRead] == 0)
    {
        return false;
    }
    else
    {
        dataR = data[nextRead][readsDone];
        readsDone++;
        nReads[nextRead]--;
        pendentReads--;
        return true;
    }
}


// Write using max latency
bool Signal::writeGen( u64bit cycle, DynamicObject* dataW )
{
    char buffer[200];

    // only lastCycle should be tested �?
    if ( lastWrite != cycle || lastCycle != cycle )
        nWrites = 0; // reset writes counter

    u32bit oldIn = in; // BTW: First time 'in' equals to 0

    in = static_cast<u32bit>(GPU_MOD( cycle, capacity ));

    u32bit index = static_cast<u32bit>(GPU_MOD( ( in + maxLatency ), capacity )); // needed here for error checking

    GPU_ASSERT(
        if ( checkReadsMissed( cycle, oldIn, in ) == 0 )
        {
            sprintf(buffer, "Error in write.  Lost data in signal \"%s\" cycle %d."
                            "DynamicObject info='%s'\n", name, cycle, (char *)dataW->getInfo());

            panic("Signal", "writeGen", buffer);
        }
        if ( lastCycle == cycle && nWrites == bandwidth )
        {
            sprintf(buffer, "Error. Max. BW (%d) exceeded (writes per cycle). Signal \"%s\" cycle %d."
                            "DynamicObject info='%s'\n", nWrites, name, cycle,
                            (char *)dataW->getInfo());

            panic("Signal", "writeGen", buffer);
        }
        if ( nReads[index] == bandwidth )
        {
            sprintf(buffer, "Error.  Max. BW exceeded (read conflict).  Signal \"%s\" cycle %d.",
                name, cycle);

            panic("Signal", "writeGen", buffer);
        }
    );

    data[index][nReads[index]] = dataW;
    nReads[index]++;

    if ( lastWrite == cycle )
        nWrites++;
    else {
        lastWrite = lastCycle = cycle;
        nWrites = 1;
    }

    return true; // OK
}


bool Signal::writeGen( u64bit cycle, DynamicObject* dataW, u32bit latency )
{
    char buffer[200];

    GPU_ASSERT( // Check for inconsistent latency, negative or greater than maxLatency
        if ( latency <= 0 || latency > maxLatency )
        {
            sprintf(buffer, "Error.  Inconsistent latency value. Signal %s, latency %d. "
                            "DynamicObject info='%s'\n", name, latency,
                            (char *)dataW->getInfo());
            panic("Signal", "writeGen", buffer);
        }
    );

    // only lastCycle should be tested
    if ( lastWrite != cycle || lastCycle != cycle ) {
        nWrites = 0;
    }

    u32bit oldIn = in; // BTW: First time 'in' equals to 0

    in = static_cast<u32bit>(GPU_MOD( cycle, capacity )); // new in

    u32bit index = static_cast<u32bit>(GPU_MOD( ( in + latency ), capacity )); // needed here for error checking

    GPU_ASSERT(
        if ( checkReadsMissed( cycle, oldIn, in ) == 0 )
        {
            sprintf(buffer, "Error in write.  Lost data in signal \"%s\" cycle %d."
                            "DynamicObject info='%s'\n",
                name, cycle, (char *)dataW->getInfo());

            panic("Signal", "writeGen", buffer);
        }
        if ( lastCycle == cycle && nWrites == bandwidth )
        {
            sprintf(buffer, "Error. Max. BW exceeded (number of writes in same cycle ). "
                            "Signal \"%s\" cycle %d. DynamicObject info='%s'\n",
                            name, cycle, (char *)dataW->getInfo());

            panic("Signal", "writeGen", buffer);
        }
        if ( nReads[index] == bandwidth )
        {
            sprintf(buffer, "Error. Max. Bw exceeded ( read conflict ). Signal \"%s\" cycle %d."
                            "DynamicObject info='%s'\n",
                             name, cycle, (char *)dataW->getInfo());

            panic("Signal", "writeGen", buffer);
        }
    );

    data[index][nReads[index]] = dataW;
    nReads[index]++;

    if ( lastWrite == cycle )
        nWrites++;
    else {
        lastWrite = lastCycle = cycle;
        nWrites = 1;
    }

    return true; // OK
}

bool Signal::readGen( u64bit cycle, DynamicObject*& dataR )
{
    char buffer[200];
    u32bit oldIn = in;
    in = static_cast<u32bit>(GPU_MOD( cycle, capacity )); // new in

    if ( lastCycle != cycle ) {
        GPU_ASSERT(
            if ( cycle < lastCycle )
            {
                sprintf(buffer, "Reading in the past in signal \"%s\" Cycle %lld. "
                                "DynamicObject info='%s'\n", 
                                name, cycle, (char *)dataR->getInfo());

                panic("Signal", "readGen", buffer);
            }
        )
        GPU_ASSERT(
            if ( checkReadsMissed( cycle, oldIn, in ) == 0 )
            {
                sprintf(buffer, "Error in read.  Lost data in signal \"%s\" cycle %lld."
                                "DynamicObject info='%s'\n",
                                name, cycle, (char *)dataR->getInfo());

                panic("Signal", "readGen", buffer);
            }
        )
        readsDone = nReads[oldIn] = 0;
    }
    lastRead = lastCycle = cycle;
    if ( nReads[in] == readsDone ) {    ;
        GPU_MESSAGE( cout << "Info: No data available yet" << endl; );
        return false; // No data available
    }
    dataR = data[in][readsDone];
    data[in][readsDone++] = 0; // cleaning... ( really not needed )
    if ( readsDone == nReads[in] )
        readsDone = nReads[in] = 0; // it was last read possible in this cycle

    return true;
}


// private ( only active is GPU_ERROR_CHECK is enabled )
bool Signal::checkReadsMissed( u64bit cycle, u32bit oldIn, u32bit newIn )
{
    u32bit i;

    // Test if there isn't outstanding data in any position of the matrix
    if ( cycle > lastCycle + maxLatency ) {
        for ( i = 0; i < capacity; i++ ) {
            if ( nReads[i] != 0 )
                return false;
        }
    }
    else { // checks partially
        if ( oldIn < newIn ) {
            for ( i = oldIn; i < newIn; i++ ) {
                if ( nReads[i] != 0 )
                    return false;
            }
        }
        else if ( oldIn > newIn ){
            for ( i = oldIn; i < capacity; i++ ) {
                if ( nReads[i] != 0 )
                    return false;
            }
            for ( i = 0; i < newIn; i++ ) {
                if ( nReads[i] != 0 )
                    return false;
            }
        }
    }
    return true; // OK
}


bool Signal::write( u64bit cycle, DynamicObject* dataW )
{
    return writeGenFast( cycle, dataW );
}

bool Signal::write( u64bit cycle, DynamicObject* dataW, u32bit lat )
{
    return writeGenFast( cycle, dataW, lat );
}

bool Signal::read( u64bit cycle, DynamicObject *&dataR )
{
    return readGenFast( cycle, dataR );
}

/*  Dumps the signal trace for this cycle and signal.  */
void Signal::traceSignal(ostream *traceFile, u64bit cycle)
{
    u32bit sigPos;
    DynamicObject *dynObj;
    u32bit numCookies;
    u32bit *cookies;
    u8bit *info;
    u32bit i, j;
    char buffer[256];

    /*  Calculate position in the signal storage array for
        the cycle to dump.  */
    sigPos = static_cast<u32bit>(GPU_MOD( cycle, capacity ));

    /*  Perhaps we should check that the cycle is OK?.  */

    /*  Read all dynamic objects stored in the signal for the cycle.  */
    for(i = 0; i < nReads[sigPos]; i++)
    {
        /*  Get a dynamic object stored in the signal storage.  */
        dynObj = (DynamicObject *) data[sigPos][i];

        /*  Write a tab, just for readibility.  */
        //fprintf(traceFile, "\t");
        (*traceFile) << "\t";

        /*  Get the dynamic object cookie list.  */
        cookies = dynObj->getCookies(numCookies);

        /*  Dump cookie list.  */
        for (j = 0; j < (numCookies - 1); j++)
        {
            //fprintf(traceFile, "%d:", cookies[j]);
            sprintf(buffer, "%d:", cookies[j]);
            (*traceFile) << buffer;
        }

        /*  Dump the last cookie without the ':' separator.  */
        //fprintf(traceFile, "%d", cookies[j]);
        sprintf(buffer, "%d", cookies[j]);
        (*traceFile) << buffer;

        /*  Write field separator.  */
        //fprintf(traceFile, ";");
        (*traceFile) << ";";

        /*  Dump the dynamic object color.  */
        //fprintf(traceFile, "%d", (u32bit) dynObj->getColor());
        sprintf(buffer, "%d", (u32bit) dynObj->getColor());
        (*traceFile) << buffer;
    
        /*  Get object info field.  */
        info = dynObj->getInfo();

        /*  Check if the info field is empty (zero string).  */
        if (info[0] != 0)
        {
            /*  Write field separator.  */
            //fprintf(traceFile, ";");
            (*traceFile) << ";";

            /*  Dump the info field.
                !!!WE HOPE IT IS JUST A NORMAL CHARACTER STRING!!!! */
            //fprintf(traceFile, "\"%s\"", info);
            (*traceFile) << "\"" << info << "\"";
        }

        /*  Use a newline for the next object in the signal.  */
        //fprintf(traceFile, "\n");
        (*traceFile) << endl;
    }
}

// inline
const char* Signal::getName() const
{
    return name;
}

// inline
u32bit Signal::getBandwidth() const
{
    return bandwidth;
}

// inline
u32bit Signal::getLatency() const
{
    return maxLatency;
}

// inline
bool Signal::isSignalDefined() const
{
    return ( ( bandwidth == 0 || maxLatency == 0 ) ? false : true );
}

// inline
bool Signal::isBandwidthDefined() const
{
    return ( bandwidth == 0 ? false : true );
}

// inline
bool Signal::isLatencyDefined() const
{
    return ( maxLatency == 0 ? false : true );
}

// inline
bool Signal::setBandwidth( u32bit newBandwidth )
{
    return ( setParameters( newBandwidth, maxLatency ) );
}

// inline
bool Signal::setLatency( u32bit newMaxLatency )
{
    return ( setParameters( bandwidth, newMaxLatency ) );
}


bool Signal::setParameters( u32bit newBandwidth, u32bit newMaxLatency )
{
    if ( newBandwidth < 0 ) // clamp
        newBandwidth = 0;
    if ( newMaxLatency < 0 )
        newMaxLatency = 0;

    if ( newBandwidth > 0 && newMaxLatency > 0 ) {
        if ( isSignalDefined() ) // destroy previous info
            destroy();

        maxLatency = newMaxLatency;
        bandwidth = newBandwidth;

        create();
        // Start of initialization of default values
        nWrites = 0;
        readsDone = 0;
        lastRead = 0;
        lastWrite = 0;
        lastCycle = 0;
        in = 0;
        nextRead = 0;
        nextWrite = maxLatency;
        pendentReads = 0;
        // End of initialization
        return true; // SIGNAL WELL-DEFINED
    }
    else { // Signal undefined
        if ( isSignalDefined() ) {
            destroy();
        }
        bandwidth = newBandwidth;
        maxLatency = newMaxLatency;
    }
    return false; // SIGNAL NOT WELL-DEFINED
}


// More efficient in future
void Signal::destroy()
{
    u32bit i;

    delete[] name;

    /*for ( i = 0; i < maxLatency; i++ )
        delete[] data[i];

    delete[] data;*/

    for(i = 0; i < capacity; i++)
    {
        delete[] data[i];
    }
    delete[] data;
    delete[] nReads;
}


// More efficient in future
void Signal::create()
{
    u32bit i,j;

    capacity = 1 << u32bit(ceil(log(f64bit(maxLatency + 1))/log(2.0)));
    capacityMask = capacity - 1;

    //data = new char**[capacity];
    data = new DynamicObject**[capacity];

    for ( i = 0; i < capacity; i++ ) {
        data[i] = new DynamicObject*[bandwidth];
        for ( j = 0; j < bandwidth; j++ ) // set mem to 0
            data[i][j] = (DynamicObject*)0; // not necesary, only for easy debugging
    }

    nReads = new u32bit[capacity];
    for ( i = 0; i < capacity; i++ ) {
        nReads[i] = 0;
    }

    /*  Check if the latency is too large for the fixed signal capacity.  */
    //if (maxLatency >= CAPACITY)
    //    panic("Signal", "create", "Maximum latency excedes maximun capacity");

    /*  Create and clear the signal buffer.  */
    //for(u32bit i = 0; i < CAPACITY; i++)
    //{
    //    data[i] = new char*[bandwidth];
    //    nReads[i] = 0;
    //}
}

