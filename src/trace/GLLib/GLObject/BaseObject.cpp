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

#include "BaseObject.h"
#include "BaseTarget.h"
#include "support.h"
#include <cstdio>
#include "GLResolver.h"
#include <iostream>

using namespace libgl;
using namespace std;

#define CHECK_PORTION(portion,funcName)\
    if ( portion >= portions )\
        panic("BaseObject", funcName, "Portion does not exists");


BaseObject::BaseObject(GLuint name, GLenum targetName, GLuint portions) : name(name), target(0), 
targetName(targetName), synchronized(false), portions(portions), clusterBitmap(portions), state(ReAlloc),
preferredMemoryHint(GPUMemory) // by default memory allocations are performed in GPU memory (not in system memory)
{
}

bool BaseObject::isSynchronized() const
{
    return ((state == Sync) || (state == Blit));
}

/*
void BaseObject::setState(State bos)
{
    state = bos;
}
*/

BaseObject::State BaseObject::getState() const
{
    return state;
}


void BaseObject::forceRealloc()
{    
    state = ReAlloc;
    clusterBitmap.clear();
    clusterBitmap.resize(portions);
}

GLuint BaseObject::getNumberOfPortions()
{
    return portions;
}

void BaseObject::setNumberOfPortions(GLuint portions)
{
    this->portions = portions;
    clusterBitmap.resize(portions);
}


GLuint BaseObject::getName() const
{
    return name;
}

GLenum BaseObject::getTargetName() const
{
    return targetName;
}

void BaseObject::setTarget(BaseTarget& bt)
{
    if ( bt.getName() != targetName )
        panic("BaseObject", "setTarget", "Target not compatible");
        
    target = &bt;    
}


BaseTarget& BaseObject::getTarget() const
{    
    if ( target == 0 )
    {
        char msg[256];
        sprintf(msg, "Object %d of type %s not attached to any target yet", name, getStringID());
        panic("BaseObject", "getTarget()", msg);
    }
        
    return *target;
}


BaseObject::~BaseObject()
{}



const char* BaseObject::getStringID() const
{
    static const char* str = "UNKNOWN Base object";
    return str;
}

string BaseObject::toString() const
{
    char tmp[256];
    sprintf(tmp, "BASE_OBJECT. Name = %d and TargetName: %s", name, 
        GLResolver::getConstantName(targetName));
    return string(tmp);    
}


void BaseObject::setTimestamp(GLuint timestamp)
{
    this->timestamp = timestamp;
}

    
GLuint BaseObject::getTimestamp() const
{
    return timestamp;
}

vector<pair<GLuint,GLuint> > BaseObject::getModifiedRanges(GLuint portion) const
{
    CHECK_PORTION(portion, "getModifiedRanges");
    
    vector<pair<GLuint,GLuint> > modifRanges;
    GLuint i = 0;
    
    const vector<bool>& bitmap = clusterBitmap[portion];
    GLuint size = bitmap.size();    
    
    bool inSequence = false;
    GLuint sequenceSize = 0;
    GLuint sequenceStart = 0;
    
    while ( i < size )
    {
        if ( !inSequence && bitmap[i] )
        {
            // start of a sequence of modified clusters
            sequenceStart = i * clusterSize;
            sequenceSize = clusterSize;
            inSequence = true;
        }
        else if ( inSequence && !bitmap[i] )
        {
            // end of a sequence of modified clusters
            modifRanges.push_back(make_pair(sequenceStart, sequenceSize));
            inSequence = false;
        }
        else if ( inSequence && bitmap[i] )
        {
            // within a sequence
            sequenceSize += clusterSize;
        }
        // else (not in a sequence)
        i++;
    }
    
    if ( inSequence )
        modifRanges.push_back(make_pair(sequenceStart, sequenceSize));
        
    return modifRanges;
}

void BaseObject::addModifiedRange(GLuint start, GLuint size, GLuint portion)
{
    
    CHECK_PORTION(portion, "addModifiedRange");

    if ( state == Blit )
        panic("BaseObject", "addModifiedRange", "Data modification in blit state not supported yet");
    
    if ( start + size > binarySize() )
        panic("BaseObject", "addModifiedRange", "Range out of bounds");
        

    if ( state == Sync ) // Update Object state if required
        state = NotSync;        
    else if ( state == ReAlloc ) 
        return ; // Object it is not allocated (or requires realloc).

    
    GLuint startCluster = start / clusterSize;
    GLuint endCluster = (start + size - 1) / clusterSize;
    
    
    vector<bool>& bitmap = clusterBitmap[portion];
    
    if ( bitmap.size() <= endCluster ) // resize cluster bitmap (preserving old values)
        bitmap.resize(endCluster + 1, false);    
                
    for ( ; startCluster <= endCluster; startCluster++ )
        bitmap[startCluster] = true; // mark as modified
        
}

void BaseObject::setBlitted()
{
    state = Blit;
}

void BaseObject::setSynchronized()
{
    clusterBitmap.clear();
    clusterBitmap.resize(portions);
    state = Sync;
}


void BaseObject::setPreferredMemoryHint(PreferredMemory memory)
{
    preferredMemoryHint = memory;
}

BaseObject::PreferredMemory BaseObject::getPreferredMemoryHint() const
{
    return preferredMemoryHint;
}

