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

#ifndef ACD_RESOURCE
    #define ACD_RESOURCE

#include "ACDTypes.h"

namespace acdlib
{

/**
 * Resource types
 */
enum ACD_RESOURCE_TYPE
{
    ACD_RESOURCE_UNKNOWN,
    ACD_RESOURCE_BUFFER,
    ACD_RESOURCE_TEXTURE1D,
    ACD_RESOURCE_TEXTURE2D,
    ACD_RESOURCE_TEXTURE3D,
    ACD_RESOURCE_TEXTURECUBEMAP,
};

/**
 * Equivalent to D3D10_USAGE enum (D3D10_USAGE_INMUTABLE has no equivalent currently)
 *
 * @note See D3D10_USAGE enum specification to a more comprensive explanation
 */
enum ACD_USAGE
{
    /**
     * Equivalent to D3D10_USAGE_DEFAULT (default if usage is not specified)
     * Not mappable, GPU in/out, 
     */
    ACD_USAGE_STATIC, ///< 
    ACD_USAGE_DYNAMIC, ///< Equivalent to D3D10_USAGE_DYNAMIC
    ACD_USAGE_STAGING ///< Equivalent to D3D10_USAGE_STAGING
};


/**
 *
 *  Defines how the resource is stored in memory.
 *
 */
enum ACD_MEMORY_LAYOUT
{
    ACD_LAYOUT_TEXTURE,
    ACD_LAYOUT_RENDERBUFFER
};


/**
 * Base interface for all resources managed by ACDResourceManager
 *
 * @note Synchronization of resources with local GPU memory is done transparently
 *       by the acdlib. Implementors on top of acdlib (OGL and D3D) can assume 
 *       instantaneuos resource data synchronization.
 *
 * @note To optimize the scheduling of resources in GPU memory the setPriority()
 *       method can be used.
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @version 1.0
 * @date 01/23/2007
 */
class ACDResource
{
public:

    /**
     * Sets the resource usage
     *
     * This method must be called before set any resource contents and can not be called
     * again after setting contents
     *
     * @param usage Resource usage
     */
    virtual void setUsage(ACD_USAGE usage) = 0;

    /**
     * Gets the resource usage
     *
     * @returns Resource usage
     */
    virtual ACD_USAGE getUsage() const = 0;

    /**
     *
     *  Sets the memory layout for the resource.
     *
     *  @param layout The memory layout defined for the resource.
     *
     */
    virtual void setMemoryLayout(ACD_MEMORY_LAYOUT layout) = 0;

    /**
     *
     *  Gets the memory layout for the resource.
     *
     *  @return The memory layout defined for the resource.
     *
     */
    virtual ACD_MEMORY_LAYOUT getMemoryLayout() const = 0;

    /**
     * Gets the resource type identifier
     *
     * @returns The resource type identifier
     */
    virtual ACD_RESOURCE_TYPE getType() const = 0;

    /**
     * Sets the priority of this resource
     *
     * @param prio the = modifiedBytes;
     *
     * @note lower number indicates greater priority (0 means maximum prority)
     */
    virtual void setPriority(acd_uint prio) = 0;

    /**
     * Gets the priority of the resource
     *
     * @returns the resource priority
     */
    virtual acd_uint getPriority() const = 0;

    /**
     * Gets the number of subresources that compound this resource
     *
     * @returns the number of subresources that compound the resource
     */
    virtual acd_uint getSubresources() const = 0;

    /**
     * Checks if a resource is well defined and can be bound to the pipeline
     *
     * @return true if the resource is well defined, false otherwise
     *
     * @note i.e: Textures are well defined when the mipmap chain is coherently defined
     */
    virtual acd_bool wellDefined() const = 0;
                         
};


}

#endif // ACD_RESOURCE
