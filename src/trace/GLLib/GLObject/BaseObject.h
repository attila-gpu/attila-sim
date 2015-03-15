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

#ifndef BASEOBJECT_H
    #define BASEOBJECT_H

#include "gl.h"
#include <vector>
#include <string>

namespace libgl
{

class BaseTarget;
    
class BaseObject
{
    
public:

    enum State
    {
        ReAlloc, // BaseObject must be reallocated or allocated for first time
        NotSync, // BaseObject must be updated
        Sync,    // BaseObject is updated
        Blit     // BaseObject updated contents are not in BaseObject data but in memory as
                 //   a result of a Blit operation.
    };
    
    enum PreferredMemory
    {
        SystemMemory,
        GPUMemory
    };


private:

    GLuint name;
    BaseTarget* target;
    const GLenum targetName; // target type supported (cannot be changed)
    GLuint portions;   
    
    /**
     * Used to identify which objects cannot be deallocated from GPUMemory because they're gonna be
     * used soon
     */
    GLuint timestamp;
    
    /**
     * Each buffer is partitioned in 1024 bytes, the minimum amount of memory that can be updated
     */
    static const int clusterSize = 1024;
    
    /**
     * This bitmap is used to keep track of BaseObject partial updates
     *
     * One vector per portion
     */
    std::vector<std::vector<bool> > clusterBitmap;
    
    State state;
        
    bool synchronized;
    
    PreferredMemory preferredMemoryHint;
    
    friend class BaseTarget;
    
protected:

    /**
     * Creates a BaseObject with a name and a targetName (type of target supported)
     *
     * This object can change its target to a new target with getName() == targetName
     */
    BaseObject(GLuint name, GLenum targetName, GLuint portions = 1);

public:

    /**
     * Sets the preferred memory where to allocate this object (it is a hint)
     *
     * @param memory can be 'GPUMemory' or 'SystemMemory'
     *
     * @note This object property defaults to GPUMemory if setPreferredMemory is not called
     */
    void setPreferredMemoryHint(PreferredMemory memory);
    
    /**
     * Gets the current preferredMemoryHint allocation for this baseObject
     *
     * @return 'GPUMemory' or 'SystemMemory'
     *
     * @note The return value is the "preferred" but not where the object will be allocated, just a hint
     */
    PreferredMemory getPreferredMemoryHint() const;
    
    /**
     * Change the state of this object to 'ReAlloc'
     *
     * Should be called each time that object contents change an require to reallocate the object
     */
    void forceRealloc();
    
    /**
     * Change the state of this object to 'Blit'
     *
     * Blit state tells the real contents of the object are in memory, not in the object internal data.
     * 
     * @note A following update of the object internal data will result in a panic. Further implementations
     *       would require to read the object data in memory before updating contents.
     *
     */
    void setBlitted();
    
    /**
     * Object synchronized
     */
    void setSynchronized();
    
    /**
     * Gets the current BaseObject state
     */
    State getState() const;
    
    virtual bool AmIDefault() { return name == 0; }
    
    /**
     * Returns the BaseTarget type/name supported by this BaseObject
     */ 
    virtual GLenum getTargetName() const;
    
    /**
     * Returns the BaseObject name
     */
    virtual GLuint getName() const;

    /**
     * Returns the current BaseTarget assigned to this BaseObject
     */    
    virtual BaseTarget& getTarget() const;    
    
    /**
     * Sets a new BaseTarget that must have the same targetName than the previous BaseTarget
     * assigned to this BaseObject
     */
    virtual void setTarget(BaseTarget& bt);

    virtual ~BaseObject() = 0;
    
    // These two calls allow to implement the copy of data into target memory (i.e GPU Memory)
    // portion parameter allows to have an object partitioned, and each partion can be queried
    // indepently (ie. texture mipmaps)
    virtual GLuint binarySize(GLuint portion = 0) = 0;    
    virtual const GLubyte* binaryData(GLuint portion = 0) = 0;
        
    
    std::vector<std::pair<GLuint,GLuint> > getModifiedRanges(GLuint portion = 0) const;
    
    /**
     * Updates internal object state
     */
    void addModifiedRange(GLuint start, GLuint size, GLuint portion = 0);
    
    // Applied to all portions
    //void resetModifiedRanges();    
    
    
    
    void setTimestamp(GLuint timestamp);
    
    GLuint getTimestamp() const;
    
    
    // set a new number of portions
    // for example, in a texture object a portion is equivalent to the number of mipmaps
    //so, if the number of mipmaps changes, the 
    void setNumberOfPortions(GLuint portions);
    
    // Must be used in subclasses to track synchronization
    // For example, when u set new contents in the object, you must call: setSynchronized(false)
    // When you copy the data into the target memory (i.e GPUMemory), then: setSynchronized(true)
    //void setSynchronized(bool sync);
    bool isSynchronized() const;

    virtual const char* getStringID() const;
    
    virtual std::string toString() const;

    GLuint getNumberOfPortions();


};    

} // namespace libgl
    
#endif // BASEOBJECT_H
