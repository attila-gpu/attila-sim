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
 * Box class implementation file.
 *
 */

#include "Box.h"
#include "StatisticsManager.h"

#include <iostream>

// definition for ListBox

using namespace gpu3d::GPUStatistics;
using namespace gpu3d;
using namespace std;

bool Box::signalTracingFlag = false;
u64bit Box::startCycle = 0;
u64bit Box::dumpCycles = 0;

void Box::setSignalTracing(bool flag, u64bit startCycle_, u64bit dumpCycles_)
{
    signalTracingFlag = flag;
    startCycle = startCycle_;
    dumpCycles = dumpCycles_;
}

bool Box::isSignalTracingRequired(u64bit cycle)
{
    if ( signalTracingFlag )
        return (startCycle <= cycle) && (cycle <= startCycle + dumpCycles - 1);
    return false;
}



StatisticsManager& Box::getSM()
{
    return sManager;
}

// Static list of boxes
Box::ListBox* Box::lBox = 0; // redundant, done by compiler



StatisticsManager& Box::sManager = StatisticsManager::instance();

// Performs basic initializatin for all boxes
Box::Box( const char* nameBox, Box* parentBox ) :
parent(parentBox), binder(SignalBinder::getBinder()), debugMode(false)
{
    // register new box
    lBox = new Box::ListBox( this, lBox );

    name = new char[strlen(nameBox)+1];
    strcpy( name, nameBox );
}


Box::~Box()
{
    // we have to remove the box from the box list
    GPU_MESSAGE(
        cout << "Trying to deleting static information of Box with name: " <<
        name << endl << "..." << endl;
    );

    ListBox* actual = lBox;
    ListBox* prev = 0;
    while ( actual ) {
        if ( strcmp( name, actual->box->name ) == 0 ) {
            if ( actual == lBox && !actual->next) {// only one box
                delete lBox;
                lBox = 0;
            }
            else if ( actual == lBox && actual->next ) {
                lBox = actual->next; // delete ListBox pointed by lBox and update lBox
                delete actual; // delete ListBox node
            }
            else
                prev->next = actual->next;

            GPU_MESSAGE(
                cout << "static info from Box" << name << "deleted successfuly" << endl;
            );

            break; // end while
        }
        prev = actual;
        actual = actual->next;
    }

    GPU_ERROR_CHECK(
        if ( !actual )
            cout << "Error. No static information found !!!!" << endl;
    );

    // delete dinamyc private contents ( name )
    delete[] name;
}

Box& Box::operator=(const Box& in)
{
    panic("Box", "operator =", "Assignment operator not supported for boxes.");

    return *this;
}



Box* Box::getBox( const char* nameBox )
{
    ListBox* actual = lBox;
    while ( actual ) {
        if ( strcmp( actual->box->name, nameBox ) == 0 )
            return actual->box;
        actual = actual->next;
    }
    return 0;
}


Signal* Box::newInputSignal( const char* name, u32bit bw,
                     u32bit latency, const char* prefix )
{
    char fullName[255];

    /*  Check if there is a prefix.  */
    if (prefix == NULL)
        sprintf(fullName, "%s", name);
    else
        sprintf(fullName, "%s::%s", prefix, name );

    return ( binder.registerSignal( fullName, SignalBinder::BIND_MODE_READ, bw, latency ) );
}


// Not tested
Signal* Box::newInputSignal( const char* name, u32bit bw, const char* prefix )
{
    return newInputSignal( name, bw, 0, prefix );
}


Signal* Box::newOutputSignal( const char* name, u32bit bw,
                     u32bit latency, const char* prefix )
{
    char fullName[255];

    /*  Check if prefix exists.  */
    if(prefix == NULL)
        sprintf(fullName, "%s", name);
    else
        sprintf( fullName, "%s::%s", prefix, name );

    return ( binder.registerSignal( fullName, SignalBinder::BIND_MODE_WRITE, bw, latency ) );
}


Signal* Box::newOutputSignal( const char* name, u32bit bw, const char* prefix )
{
    return newOutputSignal( name, bw, 0, prefix );
}

// inline
const char* Box::getName() const
{
    return name;
}


// inline
const Box* Box::getParent() const
{
    return parent;
}

void Box::getState(string &stateString)
{
    stateString = "No state info available";
}

void Box::getDebugInfo(string &debugInfo) const
{
    debugInfo = "No debug info available";
}

void Box::setDebugMode(bool mode)
{
    debugMode = mode;
}

void Box::getCommandList(string &commandList)
{
    commandList = "No box commands available";
}

void Box::execCommand(stringstream &commandStream)
{
    string command;

    commandStream >> ws;
    commandStream >> command;

    cout << "Unsupported box command >> " << command;

    while (!commandStream.eof())
    {
        commandStream >> command;
        cout << " " << command;
    }

    cout << endl;
}

//  Default function to detect stalls on boxes.
void Box::detectStall(u64bit cycle, bool &active, bool &stalled)
{
    active = false;
    stalled = false;
}

void Box::stallReport(u64bit cycle, string &stallReport)
{
    stringstream report;
    
    report << name << " stall report not available" << endl;
    
    stallReport.assign(report.str());
}

void Box::getBoxNameList(string &boxList)
{
    boxList.clear();

    ListBox* actual = lBox;

    while ( actual )
    {
        boxList.append(actual->box->name);
        boxList.append("\n");
        actual = actual->next;
    }
}

void Box::dumpNameBoxes()
{
    cout << "Boxes registered:" << endl;

    ListBox* actual = lBox;

    if ( !actual )
        cout << "No Boxes registered yet" << endl;
    while ( actual ) {
        cout << "Box name: " << actual->box->name << endl;
        actual = actual->next;
    }
}
