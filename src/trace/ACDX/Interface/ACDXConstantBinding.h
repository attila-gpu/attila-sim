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

#ifndef ACDX_CONSTANT_BINDING_H
    #define ACDX_CONSTANT_BINDING_H

#include "ACDXGlobalTypeDefinitions.h"
#include "ACDXStoredFPItemID.h"
#include "ACDXFixedPipelineState.h"
#include <vector>

namespace acdlib
{

/**
 * The ACDXConstantBinding interface allows defining the assignation/attachment 
 * of a function of several Fixed Pipeline state values and an additional direct 
 * source value to a program constant.
 *
 * This function is specified using ACDXBindingFunction subclass objects.
 *
 * @author Jordi Roca Monfort (jroca@ac.upc.edu)
 * @version 0.8
 * @date 03/16/2007
 */

/**
 * The binding constant targets
 */
enum ACDX_BINDING_TARGET
{
    ACDX_BINDING_TARGET_LOCAL,            ///< Bind a constant accesed through an "ARB" local parameter in the program
    ACDX_BINDING_TARGET_ENVIRONMENT,    ///< Bind a constant accesed through an "ARB" environment parameter in the program
    ACDX_BINDING_TARGET_FINAL,            ///< Bind a constant to the final position at the internal parameters bank.
};


/**
 * The ACDXBindingFunction interface
 *
 * Defines the interface for the constant binding function.
 */
class ACDXBindingFunction
{
public:

    /**
     * The function being called when the constant binding is resolved.
     *
     * @retval    constant        The binded output constant.
     * @param    vState            The vector of requested Fixed Pipeline states
     * @param    directSource    The optional/complementary direct source value
     */
    virtual void function(ACDXFloatVector4& constant, 
                          std::vector<ACDXFloatVector4> vState,
                          const ACDXFloatVector4& directSource) const = 0;
};

/**
 * The ACDXStateCopyBindFunction constant binding function
 *
 * This function just copies the first element of the request state
 * vector to the constant.
 *
 * @code
 *  // The function is the following:
 *    virtual void function(ACDXFloatVector4& constant, 
 *                          std::vector<ACDXFloatVector4> vState,
 *                          const ACDXFloatVector4& directSource) const
 *    {
 *        constant = vState[0];
 *    };    
 * @endcode
 */
class ACDXStateCopyBindFunction: public ACDXBindingFunction
{
public:
    virtual void function(ACDXFloatVector4& constant, 
                          std::vector<ACDXFloatVector4> vState,
                          const ACDXFloatVector4&) const
    {
        constant = vState[0];
    };
};

/**
 * The ACDXDirectSrcCopyBindFunction constant binding function.
 *
 * This function just copies the direct source to the the constant.
 *
 * @code
 *  // The function is the following:
 *    virtual void function(ACDXFloatVector4& constant, 
 *                          std::vector<ACDXFloatVector4> vState,
 *                          const ACDXFloatVector4& directSource) const
 *    {
 *        constant = directSource;
 *    };    
 * @endcode
 */
class ACDXDirectSrcCopyBindFunction: public ACDXBindingFunction
{
public:
    virtual void function(ACDXFloatVector4& constant, 
                          std::vector<ACDXFloatVector4> vState,
                          const ACDXFloatVector4& directSource) const
    {
        constant = directSource;
    };
};

/**
 * The ACDXConstantBinding interface.
 *
 * The ACDXBindingFunction interface tells the way of computing the final 
 * constant value using as a source either state contents and/or a direct source value. 
 * This ACDXBindingFunction and, in addition, the vector of requested state ids and 
 * the direct source value are passed to the ACDX API construction function 
 * (ACDXCreateConstantBinding()) that returns the corresponding ACDXConstantBinding 
 * object.
 *
 * A ACDXConstantBinding can be resolved using its resolveConstant() public
 * method or when passing it in the constant binding list to the ACDXResolveProgram()
 * function, once the internal Fixed Pipeline State has been properly set up.
 *
 * To match the constant refered in the program with the defined in the 
 * ACDXConstantBinding object, in addition to the constant index, the target stands
 * for the parameter type. In the ARB program specification, the constant parameters 
 * can be accessed as 'local parameters' (only seen by the program) or as 
 * 'environment parameters' (seen by all the programs attached to the same target). 
 * The ACDX_BINDING_TARGET enumeration allows specifying either of these targets to 
 * match the binding.
 *
 * In addition, the ACDX_BINDING_TARGET_FINAL allows defining directly the constant position 
 * in the final resolved list of constants. This target type should be used carefully because
 * the program resolver can overwrite the final constant positions due to the
 * program compilation and final constant position assignments. In fact, the use of this target
 * doesn´t make sense for "conventional" constant bindings made by the interface users and 
 * the purpose is to be used internally by the constant resolving engine.
 *
 * @note The vector of resolved states passed to the binding function can contain either
 *       single, vector and matrix states. Depending on the type each state will fill
 *       different consecutive vector positions, The convention is as follows:
 *
 * @code
 * State type specified  |  Vector positions
 * -----------------------------------------------
 *       Single          |  [0] (s,0,0,1)
 * -----------------------------------------------
 *       Vector3         |  [0] (v,v,v,1)
 * -----------------------------------------------
 *       Vector4         |  [0] (v,v,v,v)
 * -----------------------------------------------
 *       Matrix          |  [0] (m,m,m,m) (row 0)
 *                       |  [1] (m,m,m,m) (row 1)
 *                       |  [2] (m,m,m,m) (row 2)
 *                       |  [3] (m,m,m,m) (row 3)
 * -----------------------------------------------
 * @endcode
 * @endnote
 *
 * @code 
 * // EXAMPLE SAMPLES:
 * // USE1: Avoid repeating the same computation for all the program
 * //       executions. The normalization of the light direction vector
 * //       can be performed outside the shader execution.
 *
 * // This sample code binds the normalized direction to
 * // the local parameter 41.
 *
 * class NormalizeBindFunction: public ACDXBindingFunction
 * {
 * public:
 *      virtual void function(ACDXFloatVector4& constant, 
 *                            std::vector<ACDXFloatVector4> vState, 
 *                            const ACDXFloatVector4& directSource) const
 *      {
 *           constant = acdlib::_normalize(vState[0]);
 *      };
 * };
 *
 *
 * // Create the constant binding object
 * ACDXConstantBinding* cb = ACDXCreateConstantBinding(ACDX_BINDING_TARGET_LOCAL, 
 *                                                     41, ACDX_LIGHT_DIRECTION, 
 *                                                     new NormalizeBindFunction);
 *
 * 
 * // USE2: Specify the value for the environment parameter 5
 * //       accessed by the program.
 *
 * ProgramTarget pt = GET_CURRENT_VERTEX_SHADER.getTarget();
 *
 * // Create the constant binding object
 * ACDXConstantBinding* cb = ACDXCreateConstantBinding(ACDX_BINDING_TARGET_ENVIRONMENT, 
 *                                                     5, 
 *                                                     std::vector<ACDX_STORED_FP_ITEM_ID>(), // Passing an empty list
 *                                                     new ACDXDirectSourceBindFunction,
 *                                                     pt.getEnvironmentParameters()[5]);
 *
 *
 *
 * @endcode
 */

class ACDXConstantBinding
{
public:
    
    /**
     * The resolveConstant() method executes the binding function and resolves
     * the constant value using the current FP state.
     *
     * @param    fpStateImp The ACDXFixedPipelineImp object.
     * @retval    constant   The resolved constant value.        
     */
    virtual void resolveConstant(const ACDXFixedPipelineState* fpState, ACDXFloatVector4& constant) const = 0;

    /**
     * Returns the constant binding index
     *
     * @returns The constant binding index
     */
    virtual acd_uint getConstantIndex() const = 0;

    /**
     * Prints the ACDXConstantBinding object internal data state.
     *
     * @retval    os    The output stream to print to.
     */
    virtual void print(std::ostream& os) const = 0;
};

/**
 * Constant binding list definition
 */
typedef std::list<ACDXConstantBinding*> ACDXConstantBindingList;

} // namespace acdlib

#endif // ACDX_CONSTANT_BINDING_H
