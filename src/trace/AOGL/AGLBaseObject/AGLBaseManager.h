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

#ifndef AGL_BASEMANAGER
    #define AGL_BASEMANAGER

#include "gl.h"
#include "glext.h"
#include "AGLBaseTarget.h"

#include <vector>
#include <map>
#include <string>

namespace agl
{

/**
 * Base class from which all BaseManager classes must inherit
 * This class is not instanciable
 */
class BaseManager
{
private:
    
       typedef std::map<GLenum, BaseTarget*> _TargetGroup;
       typedef std::map<GLenum, BaseObject*> _BaseObjectMap;

       typedef std::map<GLenum, _BaseObjectMap*> _BaseStruct;
       // Map indexes identifies the target index that holds all the objects associated with this target
       // Map value is a pointer to a map that holds a group of objects that belong to the index map


           
    /**
     * Array of target groups
     */
    _TargetGroup* _groups;
    GLuint _nGroups; // number of target groups
    GLuint _currentGroup; // current target group

    /**
     * Map with all BaseObjects alive
     */    
    _BaseStruct _manager;
    
    /**
     * Counter maintaining the next free id
     */
    std::map<GLenum, GLuint> _nextIdMap;    
    
protected:
   
    /**
     * Computes the next free id
     *
     * @return a new unused id
     */
    GLuint findFreeId(GLuint target);
      
    /**
     * Adds a new target into the current target group
     *
     * @warning If the current group already has a target with a name equals to target->getName()
     * this method aborts
     *
     * @note This method should be called within the derived constructor
     *
     * @param target BaseTarget object
     */
    void addTarget(BaseTarget* target);

    /**
     * Constructor
     *
     * It creates all group of targets, after that calls to addTargets(), that must be implemented
     * in all subclasses and is reposible of creating all targets of all target groups. After that,
     * it creates one defautl object for each 
     *
     *
     * @param targetGroups number of target groups allowed
     */
    BaseManager(GLuint targetGroups = 1);
        
public:
    
    /**
     * Selects the active target group
     *
     * @param targetGroup one of the posible [0..numberOfGroups()-1] groups
     */ 
    void selectGroup(GLuint targetGroup);
    
    /**
     * Returns the number of target groups managed by this BaseManager
     *
     * @return the number of target groups
     */
    GLuint numberOfGroups() const { return _nGroups; }
    
    
    GLuint getCurrentGroup() const { return _currentGroup; }
    
    /**
     * Gets the target of the current group with name 'target'
     *
     * @param target the target name
     * @return the target object of the current target group with name, 'name'
     */
    virtual BaseTarget& getTarget(GLenum target) const;

    /**
     * Finds a BaseObject
     *
     * @param name BaseObject name
     */
    std::vector<BaseObject*> findObject(GLenum name);

    BaseObject* findObject(GLenum target, GLenum name);

    void addObject(GLenum target, GLenum name, BaseObject* bo);

    
    /**
     * @note Not implemented yet
     */
    virtual bool removeObjects(GLsizei n, const GLuint* names);
    
    /**
     * Binds (selects as current) the BaseObject with name 'name' for the BaseTarget with 
     * name 'target' within the current target group. If the BaseObject does not exist, it creates
     * a new one 
     * 
     * - If the BaseObject exists but the target is not compatible with the BaseTarget 
     * type (name) then this method aborts
     *
     * - If the base target does not exist in the current group this method aborts
     *
     * After call this method, if no errors have been reported, the BaseTarget with name 'target'
     * of the current Target Group is selected as the current BaseTarget of the bound BaseObject
     *     - That is: bo->setTarget(getTarget(target());
     *
     * @param target One of the targets of the current target group
     * @param name Name of the BaseObject 
     *
     * @returns The BaseObject with name 'name' having as target the BaseTarget with name 'name' of the
     *          current target group
     */
    virtual BaseObject& bindObject(GLenum target, GLuint name);
    
    /**
     * BaseManager is the owner of BaseTargets and BaseObjects, so it is responsible of clean up 
     * Basically destroys all BaseObjects and BaseTargets 
     */
    virtual ~BaseManager() = 0;

};

} // namespace agl

#endif // AGL_BASEMANAGER
