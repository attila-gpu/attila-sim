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
 * $RCSfile: SignalBinder.cpp,v $
 * $Revision: 1.14 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:44 $
 *
 * Signal Binder class implementation file.
 *
 */


#include "SignalBinder.h"
#include <sstream>
#include <iostream>

using namespace std;
using namespace gpu3d;

SignalBinder SignalBinder::binder;

const flag SignalBinder::BIND_MODE_READ = 0;
const flag SignalBinder::BIND_MODE_WRITE = 1;
const flag SignalBinder::BIND_MODE_RW = 2;


// Private ( only called once, for static initialization )
SignalBinder::SignalBinder( u32bit capacity ) : elements(0) , capacity(capacity)
{
    signals = new Signal*[capacity];
    bindingState = new flag[capacity];
}

SignalBinder& SignalBinder::getBinder()
{
    return binder;
}


Signal* SignalBinder::registerSignal( char* name, flag type, u32bit bw, u32bit latency )
{
    Signal **signalAux;
    flag *bindingAux;
    s32bit pos;
    u32bit i;
    char buff[256];

    GPU_DEBUG(
        printf("SignalBinder => Registering signal %s Bw %d Lat %d as %s\n",
            name, bw, latency, (type == BIND_MODE_READ)?"input":"output");
    )

    /*  Search the signal in the register.  */
    pos = find( name );

    /*  Check if it is a new signal.  */
    if ( pos < 0 )
    {
        /*  Register the new signal.  */

        /*  Check if there is enough space in the register.  */
        if ( elements >= capacity )
        {
            /*  Increase the signal register.  */

            GPU_DEBUG(
                printf("SignalBinder:registerSignal=> Signal Binder grows...\n");
            )

            /*  Create the new larger signal buffer.  */
            signalAux = new Signal*[capacity + growth];

            /*  Create the new  larger signal binding state buffer.  */
            bindingAux = new flag[capacity + growth];

            /*  Copy the old buffers to the new one.  */
            for (i = 0; i < capacity; i++ )
            {
                /*  Copy pointer to signals.  */
                signalAux[i] = signals[i];

                /*  Copy the signal binding state.  */
                bindingAux[i] = bindingState[i];
            }

            /*  Update signal register capacity counter.  */
            capacity = capacity + growth;

            /*  Delete the old signal buffer.  */
            delete[] signals; // destroy old array space

            /*  Delete the old signal binding state buffer.  */
            delete[] bindingState;

            /*  Set new buffer as signal buffer.  */
            signals = signalAux;

            /*  Set new buffer as binding buffer.  */
            bindingState = bindingAux;
        }

        /*  Add new signal to the signal buffer.  */
        signals[elements] = new Signal( name, bw, latency );

        /*  Set signal binding mode.  */
        if ( type == BIND_MODE_READ )
            bindingState[elements] = BIND_MODE_READ;
        else
            bindingState[elements] = BIND_MODE_WRITE;

        return signals[elements++];
    }
    // signal "fully binded" implies no more registrations allowed for this signal
    if ( bindingState[pos] == BIND_MODE_RW ) 
    {
        stringstream ss;
        ss << "No more registration allowed for signal: '" << name << "'";
        panic("SignalBinder", "registerSignal", ss.str().c_str());
    }

    // At most one read or write registration allowed
    if ( type == bindingState[pos] ) {

        if ( type == BIND_MODE_READ )
            panic("SignalBinder", "registerSignal", "Not allowed two registrations for reading in same signal.");
        else
            panic("SignalBinder", "registerSignal", "Not allowed two registrations for writing in same signal.");
    }

    // Test signal bandwidth integrity
    if ( !signals[pos]->isBandwidthDefined() ) {
        if ( bw <= 0 ) {
            panic("SignalBinder", "registerSignal", "Bandwidth must be defined.");
        }
        else // first definition for bandwidth
            signals[pos]->setBandwidth( bw );
    }
    else { // bandwidth was defined previously ( new definition must be equal )
        if ( signals[pos]->getBandwidth() != bw && bw != 0 ) {
            sprintf(buff, "No matching between actual and previous bandwidth.  Signal: %s", name);
            panic("SignalBinder", "registerSignal", buff);
        }
    }


    // Test signal latency integrity
    if ( !signals[pos]->isLatencyDefined() ) {
        if ( latency <= 0 ) {
            panic("SignalBinder", "registerSignal", "Latency must be defined.");
        }
        else // definition for latency
            signals[pos]->setLatency( latency );
    }
    else { // latency was defined previously ( new definition must be equal or 0)
        if ( signals[pos]->getLatency() != latency && latency != 0 ) {
            sprintf(buff, "No matching between actual and previous latency. Signal: %s", name);
            panic("SignalBinder", "registerSignal", buff);
        }
    }

    bindingState[pos] = BIND_MODE_RW;

    return signals[pos];
}

// inline
u32bit SignalBinder::getCapacity() const
{
    return capacity;
}

// inline
u32bit SignalBinder::getElements() const
{
    return elements;
}

Signal* SignalBinder::getSignal( const char* name ) const
{
    s32bit pos = find( name );
    return ( pos < 0 ? 0 : signals[pos] );
}

bool SignalBinder::checkSignalBindings() const
{
    for ( u32bit i = 0; i < elements; i++ ) {
        if ( bindingState[i] != BIND_MODE_RW )
            return false;
    }
    return true;
}

void SignalBinder::dump(bool showOnlyNotBoundSignals) const
{
    cout << "Capacity: " << capacity << endl;
    cout << "Elements: " << elements << endl;
    cout << "INFO: " << endl;
    if ( elements == 0 )
        cout << "NO registered signals yet" << endl;
    for ( u32bit i = 0; i < elements; i++ ) {
        // Print actual signal info
        if ( bindingState[i] != BIND_MODE_RW || !showOnlyNotBoundSignals )
            cout << signals[i]->getName();
        switch ( bindingState[i] ) {
            case BIND_MODE_READ:
                cout << "   State: R binding";
                break;
            case BIND_MODE_WRITE:
                cout << "   State: W binding";
                break;
            case BIND_MODE_RW:
                if ( !showOnlyNotBoundSignals )
                    cout << "   State: RW binding";
                break;
        }
        if ( bindingState[i] != BIND_MODE_RW || !showOnlyNotBoundSignals ) {
            cout << "   isDefined?: " << ( signals[i]->isSignalDefined() ? "DEFINED" : "NOT DEFINED" );
            cout << "   BW: " << signals[i]->getBandwidth();
            cout << "   LAT: " << signals[i]->getLatency() << endl;
        }
    }
    cout << "-------------------" << endl;
}

// Private method ( auxiliar )
s32bit SignalBinder::find( const char* name ) const
{
    for ( u32bit i = 0; i < elements; i++ ) {
        if ( strcmp( name, signals[i]->getName() ) == 0 )
            return i;
    }
    return -1;
}

/*  Start the signal trace.  */
void SignalBinder::initSignalTrace(ostream *trFile)
{
    u32bit i;
    char lineBuffer[1024];
    
    //  Set the tracefile
    traceFile = trFile;
    
    /*  Dump signal names.  */
    (*traceFile) << "Signal Trace File v. 1.0" << endl << endl;
    
    (*traceFile) << "Signal Name\t\t\tSignal ID.\tBandwidth\tLatency" << endl << endl;
    
    for(i = 0; i < elements; i++)
    {
        sprintf(lineBuffer,"%s\t\t\t%d\t\t%d\t\t%d\n", signals[i]->getName(), i,
            signals[i]->getBandwidth(), signals[i]->getLatency());
        (*traceFile) << lineBuffer;
    }

    (*traceFile) << endl << endl;
    
//    printf("\n\n");
}

/*  End the signal trace.  */
void SignalBinder::endSignalTrace()
{
    /*  Check if trace file was open.  */
    GPU_ASSERT(
        if (traceFile == NULL)
            panic("SignalBinder", "endSignalTrace", "Signal trace file was not open.");
    )

    //fprintf(traceFile,"\n\nEnd of Trace\n");
    (*traceFile) << endl << endl << "End of Trace" << endl;

    /*  Close signal trace file.  */
    //fclose(traceFile);
    //traceFile->close();
}

/*  Dump the signal trace for a given cycle.  */
void SignalBinder::dumpSignalTrace(u64bit cycle)
{
    u32bit i;
    char bufferLine[1024];

    /*  Dump the current cycle.  */
    //fprintf(traceFile,"C %ld\n", cycle);
    sprintf(bufferLine, "C %ld\n", cycle);
    (*traceFile) << bufferLine;

    /*  Dump all what is stored in all the signals for the
        current cycle.  */
    for (i = 0; i < elements; i++)
    {
        /*  Dump the signal number/id.  */
        //fprintf(traceFile, "S %d:\n", i);    
        sprintf(bufferLine, "S %d:\n", i);    
        (*traceFile) << bufferLine;
        
        /*  Dump the objects in the signal for that cycle.  */
        signals[i]->traceSignal(traceFile, cycle);
    }
}
