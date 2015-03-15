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

#include "AGLBaseManager.h"
#include "support.h"
#include <cstring>
#include <sstream>

using namespace std;
using namespace agl;

BaseManager::BaseManager(GLuint targetGroups) : _currentGroup(0), _nGroups(targetGroups)
{
    if ( _nGroups <= 0 )
        panic("BaseManager", "BaseManager", "Number of groups must be a positive number");    
    _groups = new _TargetGroup[_nGroups];        
}

BaseManager::~BaseManager()
{
    /* BaseManager is the owner of BaseTargets and BaseObjects, so it is responsible of clean up */    

    _BaseStruct::iterator target = _manager.begin();
    
    for (; target != _manager.end(); target++)
    {
        _BaseObjectMap* objectMap = target->second;
        _BaseObjectMap::iterator object = objectMap->begin();

        for (; object != objectMap->end(); object++)
            delete object->second;

        delete target->second;
    }
    
    delete[] _groups;
}

void BaseManager::selectGroup(GLuint targetGroup)
{
    if ( targetGroup >= _nGroups )
    {
        stringstream ss;
        ss << "Group " << targetGroup << " does not exist";
        panic("BaseManager", "selectGroup", ss.str().c_str());
    }
    _currentGroup = targetGroup;
}

     
GLuint BaseManager::findFreeId(GLuint target) //modificar per a que es faci a partir d'un target
{
    // zero can not be returned
    map<GLenum,GLuint>::iterator IDit = _nextIdMap.find(target);
    _BaseStruct::iterator tg = _manager.find(target);
    _BaseObjectMap* objets = tg->second;

    GLuint nextId = IDit->second;

    if ( nextId == 0 )
        nextId++;
        
    GLuint prevId = nextId - 1;
    while ( nextId != prevId )
    {
        _BaseObjectMap::iterator it = objets->begin();
        for ( ; it != objets->end(); it ++ )
        {
            if ( it->second->getName() == nextId )
                break; // id used
        }
        if ( it == objets->end() )
        {
            // nextId is free, use it
            IDit->second = nextId++;
            return nextId;
        }
        // else -> nextId already used
        nextId++; // try with an incremented id
    }
    panic("BaseManager", "findFreeId()", "All objects ids used");
    return 0;
}

      
void BaseManager::addTarget(BaseTarget* target)
{    
    _TargetGroup& tg = _groups[_currentGroup];
    _TargetGroup::iterator it = tg.find(target->getName());
    if ( it != tg.end() )
    {
        stringstream ss;
        ss << "A BaseTarget with name " << target->getName() << " already exists. Two targets with same name not allowed in same group";
        panic("BaseManager", "addTarget", ss.str().c_str());
    }
    // add target to the current group
    tg.insert(make_pair(target->getName(),target)); 

    // add target to the manager
    _BaseObjectMap* aux = new _BaseObjectMap();
    _manager.insert(make_pair(target->getName(),aux));
}
    

BaseTarget& BaseManager::getTarget(GLenum target) const
{
    _TargetGroup& targets = _groups[_currentGroup]; // get all targets of the current target group
    _TargetGroup::const_iterator it = targets.find(target); // find a target within the current target group
    
    if ( it == targets.end() )
    {
        stringstream ss;
        ss << "Target 0x" << std::hex << target << std::dec << " not found in the current target group (" 
           << _currentGroup << ")";
        panic("BaseManager", "getTarget()", ss.str().c_str());
    }

    return *(it->second); // returns the target previously found
}

vector<BaseObject*> BaseManager::findObject(GLenum name) // Eliminar
{
    _BaseStruct::iterator tg = _manager.begin();
    vector<BaseObject*> found;

    for(; tg != _manager.end(); tg++)
    {
    
        _BaseObjectMap* objectMap = tg->second;

        _BaseObjectMap::iterator object = objectMap->find(name);
        if ( object != objectMap->end() )
            found.push_back(object->second);

    }

    return found;
}

BaseObject* BaseManager::findObject(GLenum target, GLenum name)
{
    
    _BaseStruct::iterator tg = _manager.find(target); // Search if the target is inside the _manager
    if ( tg == _manager.end() )
        panic("BaseManager","bindObject","Object not found in the current target");
    
    _BaseObjectMap* objectMap = tg->second;

    _BaseObjectMap::iterator object = objectMap->find(name);
    if ( object == objectMap->end() )
        return 0;
    return object->second;

}

void BaseManager::addObject(GLenum target, GLenum name, BaseObject* bo)
{

    //Target search
    _BaseStruct::iterator tg = _manager.find(target);
    if ( tg == _manager.end() )
        panic("BaseManager","bindObject","Object not found in the current target");

    _BaseObjectMap* objectMap = tg->second;

    objectMap->insert(make_pair(name, bo)); // add BaseObject in the global BaseObject array

}


BaseObject& BaseManager::bindObject(GLenum target, GLuint name)
{    
    if ( name == 0 )
        panic("BaseManager", "bindObject", "Cannot exists an regular object with name equal to zero");
      
    BaseTarget& bt = getTarget(target); // emits a panic if the target does not exist in the current group

    BaseObject* bo = findObject(target, name);
    
    if ( bo ) // BaseObject found (previously bound)
    { 
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
            
        addObject(target,name,bo);

    }
            
    bt.setCurrent(*bo); // this target has as current (bound) this BaseObject
    return *bo;
}



bool BaseManager::removeObjects(GLsizei n, const GLuint* names)
{
    for(int i=0; i<n; i++)
    {  
        _BaseStruct::iterator target = _manager.begin();
        
        for (; target != _manager.end(); target++)
        {
            _BaseObjectMap* objectMap = target->second;
            _BaseObjectMap::iterator object = objectMap->find(names[i]);

            if ( object != objectMap->end() )
            {
                delete object->second;
                objectMap->erase(object);
            }
        }
    }
    return false;
}

