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

#ifndef ACD_ZSTENCILSTAGE
    #define ACD_ZSTENCILSTAGE

#include "ACDTypes.h"

namespace acdlib
{

/**
 * Comparison functions for Z and Stencil tests
 */
enum ACD_COMPARE_FUNCTION
{
    ACD_COMPARE_FUNCTION_NEVER, ///< Returns: 0 (false)
    ACD_COMPARE_FUNCTION_LESS, ///< Returns: Source < Destination
    ACD_COMPARE_FUNCTION_EQUAL, ///< Returns: Source == Destination
    ACD_COMPARE_FUNCTION_LESS_EQUAL, ///< Returns: Source <= Destination
    ACD_COMPARE_FUNCTION_GREATER, ///< Returns: Destination < Source
    ACD_COMPARE_FUNCTION_NOT_EQUAL, ///< Returns:  Source != Destination
    ACD_COMPARE_FUNCTION_GREATER_EQUAL, ///< Returns: Destination <= Source
    ACD_COMPARE_FUNCTION_ALWAYS ///< Returns: 1 (true)
};

/**
 * Stencil operations for the stencil test
 */
enum ACD_STENCIL_OP
{
    ACD_STENCIL_OP_KEEP, ///< Keep the existing stencil data
    ACD_STENCIL_OP_ZERO, ///< Set the stencil data to 0
    ACD_STENCIL_OP_REPLACE, ///< Set the stencil data to the reference value
    ACD_STENCIL_OP_INCR_SAT, ///< Increment the stencil value by 1, and clamp the result
    ACD_STENCIL_OP_DECR_SAT, ///< Decrement the stencil value by 1, and clamp the result
    ACD_STENCIL_OP_INVERT, ///< Invert the stencil data
    ACD_STENCIL_OP_INCR, ///< Increment the stencil value by 1, and wrap the result if necessary
    ACD_STENCIL_OP_DECR ///< Increment the stencil value by 1, and wrap the result if necessary
};

/**
 * Face identifiers
 */
enum ACD_FACE
{
    ACD_FACE_NONE, ///< undefined face
    ACD_FACE_FRONT, ///< Front face idenfier
    ACD_FACE_BACK, ///< Back face identifier
    ACD_FACE_FRONT_AND_BACK ///< front and back face identifier
};

/**
 * Interface to configure the Attila ZStencil stage
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @date 02/07/2007
 */
class ACDZStencilStage
{
public:

    /**
     * Enables or disables the Z test
     *
     * @param enable If true the Z test is enabled, otherwise the Z test is disabled
     */
    virtual void setZEnabled(acd_bool enable) = 0;

    virtual acd_bool isZEnabled() const = 0;

    /**
     * Sets the Z function used in the Z test
     *
     * @param zFunc Z function to use in the Z test
     */
    virtual void setZFunc(ACD_COMPARE_FUNCTION zFunc) = 0;

    /**
     * Gets the Z function used in the Z test
     *
     * @returns The function used to perform the Z test
     */
    virtual ACD_COMPARE_FUNCTION getZFunc() const = 0;

    /**
     * Enables or disables the Z mask
     */
    virtual void setZMask(acd_bool mask) = 0;

    /**
     * Gets if the current Z mask is enabled or disabled
     */
    virtual acd_bool getZMask() const = 0;


    /**
     * Enables or disables the Stencil test
     *
     * @param enable If true the stencil test is performed, otherwise the stencil test is omited
     */
    virtual void setStencilEnabled(acd_bool enable) = 0;

    virtual acd_bool isStencilEnabled() const = 0;

    /**
     * Sets the stencil test actions in a per-face basis
     *
     * @param face Face to apply this stencil test actions
     * @param onStencilFail Specifies the action to take when the stencil test fails
     * @param onStencilPassZFail Specifies the stencil action when the stencil test passes, 
     *                               but the depth test fails
     * @param onStencilPassZPass Specifies the stencil action when both    the stencil test and 
     *                       the depth test pass, or when the stencil test passes and either
     *                       there is no depth buffer or depth testing is not enabled
     */
    virtual void setStencilOp( ACD_FACE face, 
                               ACD_STENCIL_OP onStencilFail,
                               ACD_STENCIL_OP onStencilPassZFail,
                               ACD_STENCIL_OP onStencilPassZPass) = 0;

    virtual void getStencilOp( ACD_FACE face,
                               ACD_STENCIL_OP& onStencilFail,
                               ACD_STENCIL_OP& onStencilPassZFail,
                               ACD_STENCIL_OP& onStencilPassZPass) const = 0;

    /**
     * Sets the stencil test reference value
     *
     * @param face Face to apply this stencil function definition
     * @param func The stencil comparison function
     * @param stencilRef The stencil test reference value
     */
    virtual void setStencilFunc( ACD_FACE face, ACD_COMPARE_FUNCTION func, acd_uint stencilRef, acd_uint stencilMask ) = 0;

    /**
     * Sets the stencil range value
     *
     * @param near Specifies the mapping of the near clipping plane to window	coordinates.  The initial value	is 0
     * @param far Specifies the mapping of the far clipping plane to window	coordinates.  The initial value	is 1.
     */
    virtual void setDepthRange (acd_float near, acd_float far) = 0;

    /**
     *
     *  Sets the depth range for depth values in clip space that will be used:
     *    + [0, 1] for D3D9
     *    + [-1, 1] for OpenGL
     *
     *  @param mode Set to TRUE for D3D9 range and FALSE for OpenGL range.
     *
     */     
    virtual void setD3D9DepthRangeMode(acd_bool mode) = 0;

    /**
     * Sets the Polygon offset
     *
     * @param factor
     * @param units
     */
    virtual void setPolygonOffset (acd_float factor, acd_float units) = 0;

    /**
     * Sets the stencil update mask
     *
     * @param mask
     */
    virtual void setStencilUpdateMask (acd_uint mask) = 0;

    /**
     *
     *  Sets if the z stencil buffer has been defined.
     *
     *  @param enable Defines if the z stencil buffer is defined.
     *
     */
     
    virtual void setZStencilBufferDefined(acd_bool enable) = 0;
     
}; // class ACDZStencil

} // namespace acdlib

#endif // ACD_ZSTENCILSTAGE
