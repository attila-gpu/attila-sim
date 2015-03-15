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

#include "BaseManager.h"
#include "support.h"
#include <cstring>
#include <cstdio>

using namespace std;
using namespace libgl;

BaseManager::BaseManager(GLuint targetGroups) : nextId(1), currentGroup(0), nGroups(targetGroups)
{
    if ( targetGroups <= 0 )
        panic("BaseManager", "BaseManager", "Number of groups must be a positive number");    
    groups = new TargetGroup[nGroups];        
}

BaseManager::~BaseManager()
{
    /* BaseManager is the owner of BaseTargets and BaseObjects, so it is responsible of clean up */    
    BaseObjectMap::iterator it = baseObjs.begin();
    for ( ; it != baseObjs.end(); it++ )
        delete it->second;
    
    delete[] groups;
}

void BaseManager::selectGroup(GLuint targetGroup)
{
    if ( targetGroup >= nGroups )
    {
        char msg[256];
        sprintf(msg, "Group %d does not exist", targetGroup);
        panic("BaseManager", "selectGroup", msg);
    }
    currentGroup = targetGroup;
}

     
GLuint BaseManager::findFreeId()
{
    // zero can not be returned
    if ( nextId == 0 )
        nextId++;
        
    GLuint prevId = nextId - 1;
    while ( nextId != prevId )
    {
        BaseObjectMap::iterator it = baseObjs.begin();
        for ( ; it != baseObjs.end(); it ++ )
        {
            if ( it->second->getName() == nextId )
                break; // id used
        }
        if ( it == baseObjs.end() )
        {
            // nextId is free, use it
            return nextId++;
        }
        // else -> nextId already used
        nextId++; // try with an incremented id
    }
    panic("BaseManager", "findFreeId()", "All objects ids used");
    return 0;
}

      
void BaseManager::addTarget(BaseTarget* target)
{    
    TargetGroup& tg = groups[currentGroup];
    TargetGroup::iterator it = tg.find(target->getName());
    if ( it != tg.end() )
    {
        char msg[256];
        sprintf(msg, "A BaseTarget with name %d already exist (not allowed two targets with same name within same group)", 
                    target->getName());
        panic("BaseManager", "addTarget", msg);
    }
    // add target to the current group
    tg.insert(make_pair(target->getName(),target));    
}
    

BaseTarget& BaseManager::getTarget(GLenum target) const
{
    TargetGroup& targets = groups[currentGroup]; // get all targets of the current target group
    TargetGroup::const_iterator it = targets.find(target); // find a target within the current target group
    
    if ( it == targets.end() )
    {
        char buffer[256];
        sprintf(buffer, "Target 0x%X not found in the current target group (%d)", target, currentGroup);
        panic("BaseManager", "getTarget()", buffer);
    }

    return *(it->second); // returns the target previously found
}

BaseObject* BaseManager::findObject(GLenum name)
{
    BaseObjectMap::iterator it = baseObjs.find(name);
    if ( it == baseObjs.end() )
        return 0;
    return it->second;
}

BaseObject& BaseManager::bindObject(GLenum target, GLuint name)
{    
    if ( name == 0 )
        panic("BaseManager", "bindObject", "Cannot exists an regular object with name equal to zero");
        
    BaseTarget& bt = getTarget(target); // emits a panic if the target does not exist in the current group
    BaseObject* bo = findObject(name); 
        
    if ( bo ) // BaseObject found (previously bound)
    { 
        if ( bo->getTargetName() != target )
            panic("BaseManager", "bindObject", "Not compatible target");
        // else
        // Note that it is possible to use a target of another group if they are compatible.
        // That is, if the target selected have the same type specified in targetType()
        bo->setTarget(bt); // re-set the target (maybe it's an equivalent target of another group)
    }
    else
    {
        bo = bt.createObject(name); // each BaseTarget subclass implements its own createObject method
        
        if ( bo->getName() == 0 )
            panic("BaseManager", "bindObject", "createObject have returned an object with name equal to zero");
        
        if ( bo->getTargetName() != target ) // check creation
            panic("BaseManager", "bindObject", "Base object creation error, BaseObject targetName must be equal to target");
            
        baseObjs.insert(make_pair(name, bo)); // add BaseObject into global BaseObject array
    }
            
    bt.setCurrent(*bo); // this target has as current (bound) this BaseObject
    return *bo;
}



bool BaseManager::removeObjects(GLsizei n, const GLuint* names)
{
    for(int i=0; i<n; i++)
    {        
        BaseObjectMap::iterator it = baseObjs.find(names[i]);

        if ( it != baseObjs.end() )
        delete it->second;

        baseObjs.erase(names[i]);
    }
    
    return false;
}

