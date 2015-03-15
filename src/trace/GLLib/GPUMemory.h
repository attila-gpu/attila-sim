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

#ifndef GPUMEMORY_H
    #define GPUMEMORY_H

#include "GPUTypes.h"
#include "BaseObject.h"
#include <map>
#include <vector>

class GPUDriver; // GPUDriver is defined (for now) in the global scope

namespace libgl
{

/**
 * GPU Card memory abstraction for copy and synchronize OpenGL BaseObjects into GPU local memory
 */
class GPUMemory
{        
private:

    struct BaseObjectInfo
    {
        std::vector<GLuint> size;
        std::vector<u32bit> md;
    };
    
    std::map<BaseObject*, BaseObjectInfo> maps;

    GPUDriver* driver;
        
    void _update(BaseObject* bo, BaseObjectInfo* boi);
    void _dealloc(BaseObject* bo, BaseObjectInfo* boi);
    void _alloc(BaseObject* bo);
        
    BaseObjectInfo* _find(BaseObject* bo);
        
public:
    
    GPUMemory(GPUDriver* driver);
    
    bool allocate(BaseObject& bo);
    
    bool deallocate(BaseObject& bo);
    
    bool isPresent(BaseObject& bo);
            
    u32bit md(BaseObject& bo, GLuint portion = 0);
    
    void dump() const;
};

} // namespace libgl

#endif // GPUMEMORY_H
