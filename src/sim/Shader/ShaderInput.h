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
 * $RCSfile: ShaderInput.h,v $
 * $Revision: 1.7 $
 * $Author: vmoya $
 * $Date: 2005-06-28 10:23:42 $
 *
 * Shader Input implementation file.
 *
 */

#ifndef _SHADERINPUT_

#define _SHADERINPUT_

#include "support.h"
#include "GPUTypes.h"
#include "DynamicObject.h"

namespace gpu3d
{

/**
 *
 *  Defines the different shader inputs.
 *
 */

enum ShaderInputMode
{
    SHIM_VERTEX,                 /**<  Vertex input.  */
    SHIM_FRAGMENT,               /**<  Fragment input.  */
    SHIM_TRIANGLE,               /**<  Triangle input.  */
    SHIM_MICROTRIANGLE_FRAGMENT  /**<  Microtriangle fragment input.  */
};

struct ShaderInputID
{
    union 
    {
        struct
        {
            u32bit instance;
            u32bit index;
        } vertexID;
        
        struct
        {
            u32bit triangle;
            u32bit x;
            u32bit y;            
        } fragmentID;
        
        u32bit inputID;
    } id;
    
    ShaderInputID()
    {
        id.inputID = 0;
    }
    
    ShaderInputID(u32bit inputID)
    {
        id.inputID = inputID;
    }
    
    ShaderInputID(u32bit instance, u32bit index)
    {
        id.vertexID.instance = instance;
        id.vertexID.index = index;
    }
    
    ShaderInputID(u32bit triangle, u32bit x, u32bit y)
    {
        id.fragmentID.triangle = triangle;
        id.fragmentID.x = x;
        id.fragmentID.y = y;
    }
    
    ShaderInputID &operator=(u32bit &in)
    {
        id.inputID = in;
        
        return *this;
    }
    
    ShaderInputID(u32bit &in)
    {
        id.inputID = in;
    }
    
    bool operator==(u32bit &in) const
    {
        return (id.inputID == in);
    }
};

/**
 *
 *  This class defines objects that carry shader input
 *  and output data from and to the Shaders.
 *
 *
 */

class ShaderInput : public DynamicObject
{

private:

    ShaderInputID id;       /**<  Shader Input identifier (it uses to be its index).  */
    u32bit entry;           /**<  Destination buffer entry where is going to be stored the input after shading.  */
    u32bit unit;            /**<  Unit destination buffer where is going to be stored the input after shading.  */
    QuadFloat *attributes;  /**<  Shader Input attributes.  */
    bool kill;              /**<  Flag storing if the execution of this shader input was aborted (killed).  */
    bool isAFragment;       /**<  Stores if the shader input/output is for a fragment (TRUE) or for a vertex (FALSE).  */
    bool last;              /**<  Marks the shader input as the last in a batch (only for vertex inputs!!).  */
    ShaderInputMode mode;   /**<  Stores the shader input mode.  */
    u64bit startCycleShader;/**<  Cycle in which the shader input was issued into the shader unit.  */
    u32bit shaderLatency;   /**<  Stores the cycles spent inside the shader unit.  */

    /*  MicroPolygon Rasterizer  */
    u32bit zUnresolvedPosition;  /**< The ZUnresolved entry location.  */

public:

    /**
     *
     *  Shader Input constructor.
     *
     *  Creates and initializes a shader input.
     *
     *  @param id The shader input identifier.
     *  @param unit Unit destination buffer where is going to be stored after shading.
     *  @param entry Destination buffer entry where is going to be stored after shading.
     *  @param attrib The shader input attributes.
     *
     *  @return An initialized shader input.
     *
     */

    ShaderInput(u32bit id, u32bit unit, u32bit entry, QuadFloat *attrib);

    /**
     *
     *  Shader Input constructor.
     *
     *  Creates and initializes a shader input.
     *
     *  @param id The shader input identifier.
     *  @param unit Unit destination buffer where is going to be stored after shading.
     *  @param entry Destination buffer entry where is going to be stored after shading.
     *  @param attrib The shader input attributes.
     *  @param mode Shader input mode (vertex, fragment, triangle, ...).
     *
     *  @return An initialized shader input.
     *
     */

    ShaderInput(u32bit id, u32bit unit, u32bit entry, QuadFloat *attrib, ShaderInputMode mode);

    /**
     *
     *  Shader Input constructor.
     *
     *  Creates and initializes a shader input.
     *
     *  @param id The shader input identifier.
     *  @param unit Unit destination buffer where is going to be stored after shading.
     *  @param entry Destination buffer entry where is going to be stored after shading.
     *  @param attrib The shader input attributes.
     *
     *  @return An initialized shader input.
     *
     */

    ShaderInput(ShaderInputID id, u32bit unit, u32bit entry, QuadFloat *attrib);

    /**
     *
     *  Shader Input constructor.
     *
     *  Creates and initializes a shader input.
     *
     *  @param id The shader input identifier.
     *  @param unit Unit destination buffer where is going to be stored after shading.
     *  @param entry Destination buffer entry where is going to be stored after shading.
     *  @param attrib The shader input attributes.
     *  @param mode Shader input mode (vertex, fragment, triangle, ...).
     *
     *  @return An initialized shader input.
     *
     */

    ShaderInput(ShaderInputID id, u32bit unit, u32bit entry, QuadFloat *attrib, ShaderInputMode mode);

    /**
     *
     *  Gets the shader input identifier.
     *
     *  @return The shader input identifier.
     *
     */

    u32bit getID();

    /**
     *
     *  Gets the shader input identifier.
     *
     *  @return The shader input identifier.
     *
     */

    ShaderInputID getShaderInputID();

    /**
     *
     *  Gets the storage entry for the input.
     *
     *  @return The storage entry for the input.
     *
     */

    u32bit getEntry();

    /**
     *
     *  Gets the storage unit for the input.
     *
     *  @return The storage unit for the input.
     *
     */

    u32bit getUnit();

    /**
     *
     *  Gets the shader input attributes.
     *
     *  @return A pointer to the shader input attribute array.
     *
     */

    QuadFloat *getAttributes();

    /**
     *
     *  Marks the shader input as execution aborted/killed.
     *
     */

    void setKilled();

    /**
     *
     *  Gets the shader input kill flag.
     *
     *  @return Returns if the shader input execution was aborted (killed).
     *
     */

    bool getKill();

    /**
     *
     *  Sets shader input as a being fragment.
     *
     */

    void setAsFragment();

    /**
     *
     *  Sets shader input as a being a vertex.
     *
     */

    void setAsVertex();

    /**
     *
     *  Gets if the shader input/output is for a fragment.
     *
     *  @return If the shader input/output is for a fragment.
     *
     */
    bool isFragment();

    /**
     *
     *  Gets if the shader input/output is for a vertex.
     *
     *  @return If the shader input/output is for a vertex.
     *
     */
    bool isVertex();

    /**
     *
     *  Sets the shader (vertex) input as the last one in the batch.
     *
     *  @param last Boolean storing if the shader input is the last in the batch.
     *
     */

    void setLast(bool last);

    /**
     *
     *  Gets the last in batch mark for the shader (vertex) input.
     *
     *  @return A boolean value representing if the shader input as the last in the batch.
     *
     */

    bool isLast();

    /**
     *
     *  Get the shader input mode.
     *
     *  @return The shader input mode (fragment, vertex, ...).
     *
     */

    ShaderInputMode getInputMode();

    /**
     *
     *  Sets the start cycle in the shader.
     *
     *  @param cycle The initial cycle in the shader.
     *
     */

    void setStartCycleShader(u64bit cycle);

    /**
     *
     *  Gets the start cycle in the shader.
     *
     *  @return The start cycle in the shader.
     *
     */

    u64bit getStartCycleShader() const;

    /**
     *
     *  Sets the shader latency (cycles spent in the shader).
     *
     *  @param cycle Number of cycles the shader input spent in the shader.
     *
     */

    void setShaderLatency(u32bit cycle);

    /**
     *
     *  Gets the shader latency (cycles spent in the shader).
     *
     *  @return The number of cycles that the shader input spent inside the shader.
     *
     */

    u32bit getShaderLatency() const;
};

} // namespace gpu3d

#endif
