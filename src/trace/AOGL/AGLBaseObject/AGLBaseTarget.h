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

#ifndef AGLBASETARGET
    #define AGLBASETARGET

#include <map>
#include "AGLBaseObject.h"
#include "glext.h"

namespace agl 
{

/**
 * Base class for all classes implementing a BaseTarget
 */    
class BaseTarget
{

private:

    GLuint _target; ///< target name
    BaseObject* _current; ///< current BaseObject
    BaseObject* _def; ///< Default BaseObject for this Target
        
protected:

    friend class BaseManager; // allow access

   /**
    * Create a BaseObject with name 'name' initially bound to this target
    *
    * Only BaseManager (or a derived class) can create BaseObject calling this method
    */
       virtual BaseObject* createObject(GLuint name) = 0;
       
    
    /**
     * Mandatory constructor, called by each subclass
     */
    BaseTarget(GLuint target);

       /**
        * Sets the default object
        *
        * Often is called inside the derived constructor of this class
        */
       void setDefault(BaseObject* d);

    
public:    
          
       /**
        * Sets the default object as current
        */
       void setDefaultAsCurrent() { _current = _def; }
          
    /**
     * Gets the current BaseObject for this BaseTarget
     *
     * @return The BaseObject currently bound with this target
     */ 
    virtual BaseObject& getCurrent() const;
    
        
    /**
     * Changes the current BaseObject
     *
     * @param The new current BaseObject
     */
    virtual bool setCurrent(BaseObject& bo);
    
    /**
     * Makes undefined the current BaseObject
     */
    virtual bool resetCurrent();
    
    /**
     * Checks if a current BaseObject is defined
     *
     * @param true if exists, false otherwise
     */
    virtual bool hasCurrent() const;
    
    /**
     * Gets the name of this target
     *
     * @return a GLuint indicating the name of the target
     */
    virtual GLuint getName() const;
    
    /**
     * Destroys the default BaseObject
     */
    virtual ~BaseTarget()=0;
    
};

} // namespace libgl
    
    
#endif // AGLBASETARGET
